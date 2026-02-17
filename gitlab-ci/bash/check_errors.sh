#!/bin/bash

# check_errors.sh - Check for solver errors in the latest run directory
# This script should be run after the Slurm job has completed to verify if any solver errors
# were logged during the run. It looks for the 'solvererror' directory in the latest run logs and checks if it contains any files.

if [ -f ~/pace-bench/last_run_dir.path ]; then
  RUN_DIR_NAME=$(basename $(cat ~/pace-bench/last_run_dir.path))
  LOCAL_ERROR_DIR="stride-logs/${RUN_DIR_NAME}/solvererror"

  if [ -d "$LOCAL_ERROR_DIR" ] && [ -n "$(ls -A $LOCAL_ERROR_DIR)" ]; then
    echo "Error: solvererror non-empty in $RUN_DIR_NAME"
    exit 1
  fi
  echo "No solver errors found"
else
  echo "Warning: Could not determine run directory for error checking"
fi