#!/bin/bash
export TERM='xterm'
top -b -n 1 | sed 's/[^[:print:]\t]//g' | grep -E "COMMAND|pbs" | sed 's/\[7m//g' | sed 's/(B\[m//g' | sed 's/\[39\;49m\[K//g' | sed 's/[ /t/n]$//g'
