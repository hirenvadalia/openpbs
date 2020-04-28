#
# Copyright (C) 1994-2020 Altair Engineering, Inc.
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
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.
# See the GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Commercial License Information:
#
# For a copy of the commercial license terms and conditions,
# go to: (http://www.pbspro.com/UserArea/agreement.html)
# or contact the Altair Legal Department.
#
# Altair’s dual-license business model allows companies, individuals, and
# organizations to create proprietary derivative works of PBS Pro and
# distribute them - whether embedded or bundled with other software -
# under a commercial license agreement.
#
# Use of Altair’s trademarks, including but not limited to "PBS™",
# "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
# trademark licensing policies.
#

AC_DEFUN([PBS_AC_WITH_FLATCC],
[
  AC_ARG_WITH([flatcc],
    AS_HELP_STRING([--with-flatcc=DIR],
      [Specify the directory where flatcc is installed.]
    )
  )
  AS_IF([test "x${with_flatcc}" != "x"],
    flatcc_dir=["${with_flatcc}"],
    flatcc_dir=["/opt/flatcc"]
  )
  AC_MSG_CHECKING([for flatcc])
  AS_IF([test -r "${flatcc_dir}/bin/flatcc"],
    [flatcc_bin="${flatcc_dir}/bin/flatcc"],
    AC_MSG_ERROR([flatcc binary not found.]))
  AS_IF([test -r "${flatcc_dir}/include/flatcc/flatcc.h"],
    [flatcc_inc="-I${flatcc_dir}/include"],
    AC_MSG_ERROR([flatcc headers not found.]))
  AS_IF([test -r "${flatcc_dir}/lib/libflatcc.a"],
    [flatcc_lib="${flatcc_dir}/lib/libflatcc.a ${flatcc_dir}/lib/libflatccrt.a"],
    AC_MSG_ERROR([flatcc static library not found.])
  )
  AC_MSG_RESULT([${flatcc_dir}])
  AC_SUBST(flatcc_bin)
  AC_SUBST(flatcc_inc)
  AC_SUBST(flatcc_lib)
])
