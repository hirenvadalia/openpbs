#!/bin/bash -x

curdir=$(dirname $(readlink -f $0))
cd ${curdir}
name=$1
shift
mkdir -p /tmp/pbssetuplogs
exec ${curdir}/entrypoint "$@" &>/tmp/pbssetuplogs/${name}.log
