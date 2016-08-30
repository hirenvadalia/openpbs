# coding: utf-8

# Copyright (C) 1994-2016 Altair Engineering, Inc.
# For more information, contact Altair at www.altair.com.
#
# This file is part of the PBS Professional ("PBS Pro") software.
#
# Open Source License Information:
#
# PBS Pro is free software. You can redistribute it and/or modify it under the
# terms of the GNU Affero General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Commercial License Information:
#
# The PBS Pro software is licensed under the terms of the GNU Affero General
# Public License agreement ("AGPL"), except where a separate commercial license
# agreement for PBS Pro version 14 or later has been executed in writing with
# Altair.
#
# Altair’s dual-license business model allows companies, individuals, and
# organizations to create proprietary derivative works of PBS Pro and
# distribute them - whether embedded or bundled with other software - under
# a commercial license agreement.
#
# Use of Altair’s trademarks, including but not limited to "PBS™",
# "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
# trademark licensing policies.

import logging
import os
import re


class CliUtils(object):

    @classmethod
    def get_logging_level(cls, level):

        logging.DEBUG2 = logging.DEBUG - 1
        logging.INFOCLI = logging.INFO - 1
        logging.INFOCLI2 = logging.INFOCLI - 1

        l = None
        level = str(level).upper()
        if level == 'INFO':
            l = logging.INFO
        elif level == 'INFOCLI':
            l = logging.INFOCLI
        elif level == 'INFOCLI2':
            l = logging.INFOCLI2
        elif level == 'DEBUG':
            l = logging.DEBUG
        elif level == 'DEBUG2':
            l = logging.DEBUG2
        elif level == 'WARNING':
            l = logging.WARNING
        elif level == 'ERROR':
            l = logging.ERROR
        elif level == 'FATAL':
            l = logging.FATAL

        return l

    @staticmethod
    def check_bin(bin_name):
        ec = os.system("/usr/bin/which " + bin_name + " > /dev/null")
        if ec == 0:
            return True
        return False

    @staticmethod
    def expand_abs_path(path):
        if path.startswith('~'):
            return os.path.expanduser(path)
        return os.path.abspath(path)

    @staticmethod
    def priv_ports_info(hostname=None):
        """
        Return a list of privileged ports in use on a given host

        hostname - The host on which to query privilege ports usage. Defaults
        to the local host
        """
        from ptl.utils.pbs_dshutils import DshUtils

        netstat_tag = re.compile("tcp[\s]+[\d]+[\s]+[\d]+[\s]+"
                                 "(?P<srchost>[\w\*\.]+):(?P<srcport>[\d]+)"
                                 "[\s]+(?P<desthost>[\.\w\*:]+):"
                                 "(?P<destport>[\d]+)"
                                 "[\s]+(?P<state>[\w]+).*")
        du = DshUtils()
        ret = du.run_cmd(hostname, ['netstat', '-at', '--numeric-ports'])
        if ret['rc'] != 0:
            return False

        msg = []
        lines = ret['out']
        resv_ports = {}
        source_hosts = []
        for line in lines:
            m = netstat_tag.match(line)
            if m:
                srcport = int(m.group('srcport'))
                srchost = m.group('srchost')
                destport = int(m.group('destport'))
                desthost = m.group('desthost')
                if srcport < 1024:
                    if srchost not in source_hosts:
                        source_hosts.append(srchost)
                    msg.append(line)
                    if srchost not in resv_ports:
                        resv_ports[srchost] = [srcport]
                    elif srcport not in resv_ports[srchost]:
                        resv_ports[srchost].append(srcport)
                if destport < 1024:
                    msg.append(line)
                    if desthost not in resv_ports:
                        resv_ports[desthost] = [destport]
                    elif destport not in resv_ports[desthost]:
                        resv_ports[desthost].append(destport)

        if len(resv_ports) > 0:
            msg.append('\nPrivilege ports in use: ')
            for k, v in resv_ports.items():
                msg.append('\t' + k + ': ' +
                           str(",".join(map(lambda l: str(l), v))))
            for sh in source_hosts:
                msg.append('\nTotal on ' + sh + ': ' +
                           str(len(resv_ports[sh])))
        else:
            msg.append('No privileged ports currently allocated')

        return msg

    @staticmethod
    def parse_strace(lines):
        """
        strace parsing. Just the regular expressions for now (thanks to Carl)
        """
        timestamp_pat = r'(^(\d{2}:\d{2}:\d{2})(.\d+){0,1} |^(\d+.\d+) ){0,1}'
        exec_pat = r'execve\(("[^"]+"), \[([^]]+)\], [^,]+ = (\d+)$'
        timestamp_exec_re = re.compile(timestamp_pat + exec_pat)
        for line in lines:
            m = timestamp_exec_re.match(line)
            if m:
                print line
