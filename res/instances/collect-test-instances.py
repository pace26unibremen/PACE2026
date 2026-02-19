#!/usr/bin/env python3
import json
import sys
import argparse
from collections import defaultdict, Counter
import math

# =============================================================================
# USER VARIABLES - Tune these easily!
# =============================================================================
"""
All tunable parameters are here. Adjust without touching code below.

TIME_BUCKETS (dict[str, float]): Exclusive time thresholds (s).
  - Keys: category names (used in stats/output).
  - Values: upper bound (< thresh).
  - Adds implicit 'slow' > last threshold.
  Example: {'fast1s':1.0, 'fast30s':30.0} -> <1s, 1-30s, >=30s.

SAMPLE_FRACS (dict[str, float]): Sampling fraction per time category.
  - Keys: match TIME_BUCKETS keys.
  - Values: fraction (0.0-1.0); min 1 instance.
  - Samples slowest first (reverse sorted by wtime).
  - 'fast1s' implicitly 1.0 (all).

SMALL_LEAVES_MAX (int): Max leaves for "hard small" category.

HARD_SMALL_VALID_N (int): Num slowest Valid <= SMALL_LEAVES_MAX.
HARD_SMALL_TIMEOUT_N (int): Num smallest Timeout <= SMALL_LEAVES_MAX.

BUCKET_START (int), BUCKET_FACTOR (float): Leaf bucketing.
  - Buckets: START, START*FACTOR, START*FACTOR^2, ...
  - E.g., 50*2 -> 50,100,200,400,...

BUCKET_SIZES (dict[int, int]): Max instances per bucket.
  - Prefers Timeout + slowest Valid (half each).

HUGE_LEAVES_MIN (int), HUGE_TIME_MAX (float): Detect fast-solved giants.
"""

# Time thresholds (seconds)
TIME_BUCKETS = {
    'fast1s': 1.0,           # All <1s
    'fast30s': 30.0,         # Sample up to 30s
    'fast5m': 300.0,         # Sample up to 5min
    'fast15m': 900.0,        # Sample up to 15min
}

# Sampling fractions (for non-all buckets)
SAMPLE_FRACS = {
    'fast1s': 1.0,           # All <1s
    'fast30s': 0.20,         # 20% of 1-30s (slowest)
    'fast5m': 0.05,          # 5% of 30s-5m (slowest)
    'fast15m': 0.01,         # 1% of 5-15m (slowest)
}

# Sampling fractions for SLOW categories only (>=15m)
SLOW_SAMPLE_FRACS = {
    'fast15m': 0.10,         # 10% of 5-15m (more than before)
    'slow': 0.20,            # 20% of >15m
}

# Hard small instances (<= small_leaves_max)
SMALL_LEAVES_MAX = 35
HARD_SMALL_VALID_N = 10      # Top N slowest Valid <= small_leaves_max
HARD_SMALL_TIMEOUT_N = 20    # Top N smallest Timeout <= small_leaves_max

# Bucket config for larger instances
BUCKET_START = 50
BUCKET_FACTOR = 2            # 50,100,200,...
BUCKET_SIZES = {             # Override per-bucket if needed
    50: 20,
    100: 10,
    200: 5,
    400: 3,
    800: 2,
    1600: 1,
}

# Huge solved detection
HUGE_LEAVES_MIN = 400
HUGE_TIME_MAX = 3600         # Solved <1h

# =============================================================================
# FUNCTIONS (with docstrings, types)
# =============================================================================

def load_summary(filename: str) -> list[dict]:
    """
    Load JSON lines from summary file.

    Args:
        filename (str): Path to summary.json.

    Returns:
        list[dict]: List of instance dicts.

    Raises:
        FileNotFoundError, json.JSONDecodeError.
    """
    data = []
    with open(filename, 'r') as f:
        for line in f:
            data.append(json.loads(line.strip()))
    return data

def categorize_by_time(valid_data: list[dict], time_buckets: dict[str, float]) -> dict[str, list[dict]]:
    """
    Categorize valid instances into exclusive time buckets (slowest first).

    Args:
        valid_data (list[dict]): Valid instances only.
        time_buckets (dict[str, float]): {cat_name: upper_thresh}.

    Returns:
        dict[str, list[dict]]: {cat: sorted_list (desc wtime)}.
        Includes 'slow' for > max threshold.
    """
    categories = {name: [] for name in time_buckets}
    prev_thresh = 0.0
    for name, thresh in time_buckets.items():
        cat = [d for d in valid_data if prev_thresh <= d['s_wtime'] < thresh]
        categories[name] = sorted(cat, key=lambda d: d['s_wtime'], reverse=True)
        prev_thresh = thresh
    # Fast: ALL < max threshold (combined)
    max_fast = list(time_buckets.values())[-1]
    categories['fast_all'] = [d for d in valid_data if d['s_wtime'] < max_fast]
    # Slow (> last threshold)
    categories['slow'] = [d for d in valid_data if d['s_wtime'] >= max_fast]
    return categories

def sample_slow_categories(categories: dict[str, list[dict]], sample_fracs: dict[str, float]) -> list[dict]:
    """
    Sample slowest from SLOW categories only (>=15m).

    Args:
        categories (dict[str, list[dict]]): From categorize_by_time.
        sample_fracs (dict[str, float]): {cat: frac} for slow cats.

    Returns:
        list[dict]: Flattened selected.
    """
    selected = []
    for cat_name, frac in sample_fracs.items():
        if cat_name in categories and cat_name != 'fast_all':
            n_sample = max(1, int(frac * len(categories[cat_name])))
            selected.extend(categories[cat_name][:n_sample])
    return selected


def sample_categories(categories: dict[str, list[dict]], sample_fracs: dict[str, float]) -> list[dict]:
    """
    Sample slowest from categories.

    Args:
        categories (dict[str, list[dict]]): From categorize_by_time.
        sample_fracs (dict[str, float]): {cat: frac}.

    Returns:
        list[dict]: Flattened selected (may have dups, dedup later).
    """
    selected = []
    for cat_name, frac in sample_fracs.items():
        if cat_name in categories:
            n_sample = max(1, int(frac * len(categories[cat_name])))
            selected.extend(categories[cat_name][:n_sample])
    return selected

def get_bucket(leaves: int, start: int = 50, factor: float = 2) -> int:
    """
    Compute leaf bucket.

    Args:
        leaves (int): s_num_leaves.
        start, factor: Bucketing params.

    Returns:
        int: Bucket value (or 'small' if <=35).
    """
    bucket = start
    while bucket < leaves:
        bucket = math.ceil(bucket * factor)
    return bucket
def select_hard_small(data: list[dict], small_leaves_max: int,
                      n_valid: int, n_timeout: int) -> list[dict]:
    """
    Hard small: slowest Valid + smallest Timeouts <= small_leaves_max.

    Args:
        data (list[dict]): All data.
        small_leaves_max (int), n_valid (int), n_timeout (int).

    Returns:
        list[dict]: Combined selection.
    """
    valid_small = [d for d in data if d['s_result'] == 'Valid' and d['s_num_leaves'] <= small_leaves_max]
    timeout_small = [d for d in data if d['s_result'] == 'Timeout' and d['s_num_leaves'] <= small_leaves_max]
    hard_valid = sorted(valid_small, key=lambda d: d['s_wtime'], reverse=True)[:n_valid]
    hard_timeout = sorted(timeout_small, key=lambda d: d['s_num_leaves'])[:n_timeout]
    return hard_valid + hard_timeout

def select_buckets(data: list[dict], bucket_sizes: dict[int, int]) -> tuple[list[dict], dict]:
    """
    Select from leaf buckets (Timeout + slow Valid).

    Args:
        data (list[dict]): All data.
        bucket_sizes (dict[int,int]): {bucket: max_n}.

    Returns:
        list[dict]: Selected across buckets.
    """
    buckets = defaultdict(list)
    for d in data:
        b = get_bucket(d['s_num_leaves'])  # Now always int >=50
        if b >= BUCKET_START:  # Skip tiny
            buckets[b].append(d)

    selected = []
    bucket_stats = {}
    print("\nLeaf buckets:")
    for b in sorted(buckets.keys()):  # Safe: all int
        bucket_data = buckets[b]
        n_total = len(bucket_data)
        timeouts = [d for d in bucket_data if d['s_result'] == 'Timeout']
        n_timeouts = len(timeouts)
        valid_bucket = [d for d in bucket_data if d['s_result'] == 'Valid']
        slow_valid = sorted(valid_bucket, key=lambda d: d['s_wtime'], reverse=True)
        print(f"  {b}: {n_total} total ({n_timeouts} timeout)")

        size = bucket_sizes.get(b, 1)
        bucket_sel = timeouts[:size//2] + slow_valid[:size//2]
        selected.extend(bucket_sel[:size])
        bucket_stats[b] = {'total': n_total, 'timeouts': n_timeouts, 'selected': len(bucket_sel)}

    return selected, bucket_stats
def find_huge_solved(valid_data: list[dict], huge_leaves_min: int,
                     huge_time_max: float) -> list[dict]:
    """Fast-solved huge instances."""
    huge = [d for d in valid_data
            if d['s_num_leaves'] >= huge_leaves_min and d['s_wtime'] <= huge_time_max]
    return sorted(huge, key=lambda d: d['s_num_leaves'], reverse=True)[:5]

def dedup(selected: list[dict]) -> list[dict]:
    """Deduplicate by s_idigest."""
    seen = set()
    unique = []
    for d in selected:
        key = d['s_idigest']
        if key not in seen:
            seen.add(key)
            unique.append(d)
    return unique

def print_stats(data: list[dict], valid_data: list[dict]):
    """Print data overview."""
    print(f"Loaded {len(data)} entries ({len(valid_data)} valid).")
    leaves = [d['s_num_leaves'] for d in data]
    times = [d['s_wtime'] for d in valid_data]
    print(f"Leaves: min={min(leaves)}, max={max(leaves)}, median={sorted(leaves)[len(leaves)//2]}")
    if times:
        print(f"Times: min={min(times):.2f}s, median={sorted(times)[len(times)//2]:.2f}s, max={max(times):.2f}s")

def write_lst(selected: list[dict], output_file: str) -> None:
    """Write s:ID lines to file."""
    with open(output_file, 'w') as f:
        for d in selected:
            f.write(f"s:{d['s_idigest']}\n")

def main(summary_file: str, output_file: str) -> None:
    """
    Main workflow.
    """
    data = load_summary(summary_file)
    valid_data = [d for d in data if d['s_result'] == 'Valid']

    print_stats(data, valid_data)

    # Time sampling: ALL FAST + sampled slow
    categories = categorize_by_time(valid_data, TIME_BUCKETS)
    print("Time categories:")
    for name, cat in categories.items():
        print(f"  {name}: {len(cat)}")

    fast_selected = categories['fast_all']  # ALL <15m (~4869)
    slow_sampled = sample_slow_categories(categories, SLOW_SAMPLE_FRACS)  # Sampled slow

    # Hard small
    hard_small = select_hard_small(data, SMALL_LEAVES_MAX, HARD_SMALL_VALID_N, HARD_SMALL_TIMEOUT_N)

    # Buckets w/ stats
    bucket_selected, bucket_stats = select_buckets(data, BUCKET_SIZES)
    print("\nBucket selection summary:")
    for b, stats in bucket_stats.items():
        print(f"  {b}: {stats['selected']}/{stats['total']} ({stats['timeouts']} timeout)")

    # Combine & dedup
    selected = dedup(fast_selected + slow_sampled + hard_small + bucket_selected)

    # Huge solved
    huge = find_huge_solved(valid_data, HUGE_LEAVES_MIN, HUGE_TIME_MAX)
    if huge:
        print("\nHuge solved (<1h, >=400 leaves):")
        for d in huge:
            print(f"  {d['s_idigest']}: {d['s_num_leaves']}l, {d['s_wtime']:.0f}s")

    write_lst(selected, output_file)
    print(f"\nSelected {len(selected)} unique -> {output_file}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Filter Stride summary.json to .lst")
    parser.add_argument("summary", help="summary.json path")
    parser.add_argument("output", help=".lst output path")
    args = parser.parse_args()
    main(args.summary, args.output)
