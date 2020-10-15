#!/bin/bash

if [ ! -d /tmp/pbs ]; then
	exit 0
fi

if [ "x$(ls -1 /tmp/pbs 2>/dev/null | grep pbs-server)" == "x" ]; then
	exit 0
fi

truncate -cs 0 /tmp/pbs/pbs-server-*/server_logs/*
truncate -cs 0 /tmp/pbs/pbs-server-*/sched_logs/*
truncate -cs 0 /tmp/pbs/pbs-server-*/server_priv/accounting/*
