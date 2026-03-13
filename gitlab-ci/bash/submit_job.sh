#!/bin/bash

# submit_job.sh - Submit a new HPC job for the PACE benchmark

cd ~/pace-bench || exit

# Backup stride-logs if it exists
if [ -d "hpc_transfer/stride-logs" ]; then
  mv hpc_transfer/stride-logs stride-logs.backup
fi

tar xzf hpc_payload.tgz

# Restore stride-logs
if [ -d "stride-logs.backup" ]; then
  mv stride-logs.backup hpc_transfer/stride-logs
fi

# Record the old 'latest' target before submitting
OLD_LATEST=""
if [ -L "hpc_transfer/stride-logs/latest" ]; then
  OLD_LATEST=$(readlink -f hpc_transfer/stride-logs/latest)
  echo "$OLD_LATEST" > old_run_dir.path
fi

# Now submit the job
cd hpc_transfer || exit
chmod +x *.sh
chmod +x *.slurm
JOB_ID=$(/usr/bin/sbatch run_stride.slurm | awk '{print $4}')
cd ..
echo "$JOB_ID" > last_job.id
echo "Submitted job $JOB_ID"