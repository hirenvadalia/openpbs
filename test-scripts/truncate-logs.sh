#!/bin/bash

if [ ! -d /var/spool/pbs ]; then
	exit 0
fi

if [ "x$(ls -1 /var/spool/pbs 2>/dev/null | grep -E 'pbs-server|pbs-mom')" == "x" ]; then
	exit 0
fi

truncate -cs 0 /var/spool/pbs/pbs-*/*_logs/*
truncate -cs 0 /var/spool/pbs/pbs-server-*/server_priv/accounting/*
