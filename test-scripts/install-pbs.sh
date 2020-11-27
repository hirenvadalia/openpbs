#!/bin/bash

curdir=$(dirname $(readlink -f $0))
cd ${curdir}

dnf install -y ${curdir}/openpbs-server.rpm
rm -rf /etc/pbs.conf* /var/spool/pbs
