#!/bin/bash

_wrokd=$1
if [ "x${_workd} == "x" -o ! -d "${_workd} ]; then
	exit 0
fi

if [ "x$(ls -1 "${_workd} 2>/dev/null | grep -E 'pbs-server|pbs-mom')" == "x" ]; then
	exit 0
fi

truncate -cs 0 ${_workd}/pbs-*/*_logs/*
truncate -cs 0 ${_workd}/pbs-server-*/server_priv/accounting/*
