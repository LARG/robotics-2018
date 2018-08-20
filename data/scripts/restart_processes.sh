#!/bin/sh

processes="vision
motion"
while [ 1 ]
do
  for p in $processes 
  do
    if [ ! "$(pidof $p)" ] 
    then
      echo -e "\n\n***********\nRESTARTING $p PROCESS\n**********\n"
      /home/nao/bin/$p &
    fi
  done
  # FYI: 1 second is too fast and will slow down the processor. 10 should be fast enough.
  sleep 10
done
