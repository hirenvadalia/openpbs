#!/bin/bash -x

_phome=/tmp/pbs

if [ ! -d /opt/pbs -o ! -d ${_phome}/confs ]; then
	exit 0
fi

for _name in $(ls -1 ${_phome} 2>/dev/null | grep -E 'pbs-server|pbs-mom')
do
	export PBS_CONF_FILE=${_phome}/confs/${_name}.conf
	. ${_phome}/confs/${_conf}.conf
	if [ ! -d ${PBS_HOME} ]; then
		continue
	fi
	/opt/pbs/libexec/pbs_init.d stop
done
