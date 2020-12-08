#!/bin/bash

_curdir=$(dirname $(readlink -f $0))
_self=${_curdir}/$(basename $0)
cd ${_curdir}

if [ $# -gt 5 ]; then
	echo "syntax: $0 <no. jobs> [j|ja] <no. subjob if ja>"
	exit 1
fi

if [ "x$1" == "xsubmit" ]; then
	nocon=$2
	njobs=$3
	jtype=$4
	nsj=$5

	if [ "x${nocon}" == "x0" ]; then
		. /etc/pbs.conf
	else
		. /var/spool/pbs/confs/pbs-server-1.conf
		export PBS_CONF_FILE=/var/spool/pbs/confs/pbs-server-1.conf
	fi

	if [ "x${jtype}" == "xja" ]; then
		for i in $(seq 1 $njobs)
		do
			/opt/pbs/bin/qsub -J1-${nsj} -koe -- /bin/date > /dev/null
		done
	else
		for i in $(seq 1 $njobs)
		do
			/opt/pbs/bin/qsub -koe -- /bin/date > /dev/null
		done
	fi
	exit 0
fi

if [ "x$1" == "xsvr-submit" ]; then
	nocon=$2
	njobs=$3
	jtype=$4
	nsj=$5
	users=$(awk -F: '/^(pbsu|tst)/ {print $1}' /etc/passwd)
	_tu=$(awk -F: '/^(pbsu|tst)/ {print $1}' /etc/passwd | wc -l)
	_jpu=$(( njobs / _tu ))

	for usr in ${users}
	do
		( setsid sudo -Hiu ${usr} ${_self} submit ${nocon} ${_jpu} ${jtype} ${nsj}) &
	done
	wait
	exit 0
fi

nocon=$1
njobs=$2
jtype=$3
nsj=$4
if [ "x${jtype}" == "xja" ]; then
	echo "Total jobs array: ${njobs}, Total Subjob per array: ${nsj}, Total svrs: 1, Jobs array per Svr: ${njobs}"
else
	echo "Total jobs: ${njobs}, Total svrs: 1, Jobs per Svr: ${njobs}"
fi
if [ "x${nocon}" == "x0" ]; then
	podman exec pbs-server-1 ${_self} svr-submit ${nocon} ${njobs} ${jtype} ${nsj}
else
	${_self} svr-submit ${nocon} ${njobs} ${jtype} ${nsj}
fi
