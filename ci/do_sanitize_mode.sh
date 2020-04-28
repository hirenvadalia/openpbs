#!/bin/bash -xe

PBS_DIR=$( readlink -f $0 | awk -F'/ci/' '{print $1}' )
cd ${PBS_DIR}

yum clean all
yum -y update
yum -y install yum-utils epel-release rpmdevtools libasan llvm
rpmdev-setuptree
yum -y install python3-pip sudo which net-tools man-db time.x86_64 cmake wget
yum-builddep -y ./*.spec
wget https://github.com/dvidelabs/flatcc/archive/master.tar.gz
tar -xf master.tar.gz
cd flatcc-master
mkdir build
cd build
cmake -DFLATCC_TEST=OFF -DFLATCC_CXX_TEST=OFF -DBUILD_SHARED_LIBS=OFF ..
make
mkdir /opt/flatcc
cp -rf ../bin ../lib ../include /opt/flatcc
cd ${PBS_DIR}
./autogen.sh
rm -rf target-sanitize
mkdir -p target-sanitize
cd target-sanitize
../configure
make dist
cp -fv *.tar.gz /root/rpmbuild/SOURCES/
CFLAGS="-g -O2 -Wall -Werror -fsanitize=address -fno-omit-frame-pointer" rpmbuild -bb --with ptl *.spec
yum -y install /root/rpmbuild/RPMS/x86_64/*-server-??.*.x86_64.rpm
yum -y install /root/rpmbuild/RPMS/x86_64/*-debuginfo-??.*.x86_64.rpm
yum -y install /root/rpmbuild/RPMS/x86_64/*-ptl-??.*.x86_64.rpm
sed -i "s@PBS_START_MOM=0@PBS_START_MOM=1@" /etc/pbs.conf
/etc/init.d/pbs start
set +e
. /etc/profile.d/ptl.sh
set -e
pbs_config --make-ug
cd /opt/ptl/tests/
pbs_benchpress --tags=smoke
