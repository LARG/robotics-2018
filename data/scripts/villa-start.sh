#!/bin/bash

source /etc/profile

memory_found=false

# stop old wireless-monitor
pid=`top -n1 -b | grep python | grep wireless-monitor | grep -o -E '^\s*[0-9]+'`
kill -9 $pid &> /dev/null
# start a new one
#/home/nao/bin/wireless-monitor.py &

for i in {1..60}
do
  /home/nao/bin/memory_test
  if [ $? -eq 0 ]; then
    memory_found=true
    break
  fi
  sleep 1
done

if $memory_found; then
  /home/nao/bin/motion &
  sleep 1
  LD_LIBRARY_PATH=/home/$USER/bin
  /home/nao/bin/vision 2>&1 | tee -a visionoutput.txt &
  sleep 1
  /home/nao/bin/restart_processes.sh &
else
  echo "Live Memory Not Found: NOT STARTING MOTION OR VISION"
fi
