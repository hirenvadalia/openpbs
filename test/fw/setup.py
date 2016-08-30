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

from setuptools import setup, find_packages
import os

os.chdir(os.path.dirname(os.path.abspath(os.path.abspath(__file__))))


def get_reqs():
    install_requires = open('requirements.txt').readlines()
    return [r.strip() for r in install_requires]


def get_scripts():
    return ['bin/%s' % (x) for x in os.listdir('bin')]

setup(
    name='PbsTestLab',
    version='1.0.0',
    author='Vincent Matossian',
    author_email='pbspro@groups.io',
    packages=find_packages(),
    scripts=get_scripts(),
    url='http://www.pbspro.com',
    license='AGPLv3 with exceptions',
    description='PBS Pro Testing and Benchmarking Framework',
    long_description=open('README.txt').read(),
    install_requires=get_reqs(),
    keywords='PbsTestLab ptl pbspro',
    zip_safe=False,
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Other Environment',
        'Intended Audience :: Developers',
        'License :: AGPLv3 with exceptions',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Topic :: Software Development :: Testing',
        'Topic :: Software Development :: Quality Assurance',
    ]
)

# run setexecutionpolicy remotesigned
# run winrm quickconfig -q
