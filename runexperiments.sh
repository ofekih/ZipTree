#! /usr/bin/env bash

pkill -u ogila -f test

hostname="$(hostname)"
# short_hostname="${hostname}"
short_hostname="${hostname%.ics.uci.edu}"

for (( i=$1; i<=$2; i++ ))
do
	nohup ./test "${short_hostname}.${i}" &
done
