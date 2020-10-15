#!/bin/bash

curdir=$(dirname $(readlink -f $0))
cd ${curdir}

resultdir=${curdir}/results/$1
num_jobs=$2
jtype=$3
num_subjobs=$4
CON_CMD="podman exec pbs-server-1"
SSH_CMD="ssh -o ControlMaster=auto -o ControlPersist=300 -o ControlPath=~/.ssh/.cm-%r@%h@%p -o StrictHostKeyChecking=no"
SCP_CMD="scp -o ControlMaster=auto -o ControlPersist=300 -o ControlPath=~/.ssh/.cm-%r@%h@%p -o StrictHostKeyChecking=no"

###############################
# utility funcs
###############################
function collect_info() {
	local _host

	for _host in $(cat ${curdir}/nodes)
	do
		echo "--- ${_host}"
		if [ "x${_host}" == "x$(hostname)" -o "x${_host}" == "x$(hostname -f)" ]; then
			uptime
			free -hw
		else
			${SSH_CMD} ${_host} uptime
			${SSH_CMD} ${_host} free -hw
		fi
		echo "------"
	done
}

function wait_jobs() {
	local _ct _last=0 _nct=0

	while true
	do
		_ct=$(${CON_CMD} /opt/pbs/bin/qstat -Bf 2>/dev/null | grep total_jobs | awk -F' = ' '{ print $2 }')
		if [ "x${_ct}" == "x0" ]; then
			echo "total_jobs: ${_ct}"
			break
		elif [ "x${_ct}" == "x${_last}" ]; then
			if [ ${_nct} -gt 10 ]; then
				echo "Looks like job counts is not changing, force exiting wait loop"
				break
			fi
			_nct=$(( _nct + 1 ))
		else
			_last=${_ct}
			_nct=0
		fi
		echo "total_jobs: ${_ct}"
		collect_info
		sleep 10
	done
}

function collect_logs() {
	local _host _svrs _d _fsvr="" _destd=${resultdir}/$1

	mkdir -p ${_destd}
	for _host in $(cat ${curdir}/nodes)
	do
		_svrs=$(${SSH_CMD} ${_host} ls -1 /tmp/pbs 2>/dev/null | grep pbs-server | sort -u)
		if [ "x${_svrs}" == "x" ]; then
			continue
		fi
		for _d in ${_svrs}
		do
			echo "Saving logs from ${_d}"
			rm -rf ${_destd}/${_d}
			mkdir -p ${_destd}/${_d}
			if [ -d /tmp/pbs/${_d} ]; then
				cp -rf /tmp/pbs/${_d}/server_logs ${_destd}/${_d}/
				cp -rf /tmp/pbs/${_d}/server_priv/accounting ${_destd}/${_d}/
				if [ "x${_fsvr}" == "x" ]; then
					cp -rf /tmp/pbs/${_d}/sched_logs ${_destd}/${_d}/
					_fsvr=${_d}
				fi
			else
				${SCP_CMD} -qr ${_host}:/tmp/pbs/${_d}/server_logs ${_destd}/${_d}/
				${SCP_CMD} -qr ${_host}:/tmp/pbs/${_d}/server_priv/accounting ${_destd}/${_d}/
				if [ "x${_fsvr}" == "x" ]; then
					${SCP_CMD} -qr ${_host}:/tmp/pbs/${_d}/sched_logs ${_destd}/${_d}/sched_logs
					_fsvr=${_d}
				fi
			fi
		done
		${SSH_CMD} ${_host} ${curdir}/truncate-logs.sh
	done
}
###############################
# end of utility funcs
###############################


############################
# test funcs
############################
function test_with_sched_off() {
	${CON_CMD} /opt/pbs/bin/qmgr -c "s s scheduling=0"
	${curdir}/submit-jobs.sh ${num_jobs} ${jtype} ${num_subjobs}
	${CON_CMD} /opt/pbs/bin/qmgr -c "s s scheduling=1"
	wait_jobs
	collect_logs sched_off
}

function test_with_sched_on() {
	${CON_CMD} /opt/pbs/bin/qmgr -c "s s scheduling=1"
	${curdir}/submit-jobs.sh ${num_jobs} ${jtype} ${num_subjobs}
	wait_jobs
	collect_logs sched_on
}
############################
# end test funcs
############################
collect_info
test_with_sched_off
collect_info
test_with_sched_on
