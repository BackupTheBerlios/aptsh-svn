#!/bin/sh
# Author: Marcin Wrochniak <wrochniak@gmail.com>
# Distributed under GPL
# Script for building Aptsh packages.
# Usage:
# "./build-aptsh.sh svn" - if you want the hottest version from Subversion repository
# "./build-aptsh.sh 0.0.1" - if you want to build 0.0.1 version (you must be in directory with unpacked source release)
# "./build-aptsh.sh release 0.0.1" - grab sources from svn and build a release package

if [ $1 = 'svn' ]; then
	svn checkout svn://svn.berlios.de/aptsh/trunk/
	mv trunk/* .
	rm -rf trunk
	f=aptsh-`date +%Y.%m.%d`
elif [$1 = 'release' ]
	svn checkout svn://svn.berlios.de/aptsh/trunk/
	mv trunk/* .
	rm -rf trunk
	f=$2
else
	f=$1
fi

mkdir $f

cp -rf * $f/

cd $f

dh_make -s -e wrochniak@gmail.com

perl -pi -e 's/^Maintainer.*$/Maintainer: Marcin Wrochniak <wrochniak\@gmail\.com>/g' debian/control
perl -pi -e 's/^Description.*$/Description: APT interactive shell/g' debian/control
cat debian/control | head -n $[`wc -l debian/control | awk '{print $1}'`-1] > control_tmp
cp control_tmp debian/control
echo " Aptsh will help you in managing packages by providing very nice pseudo-shell." >> debian/control
rm control_tmp

echo "/etc/aptsh.conf" > debian/conffiles

echo "etc"  >> debian/dirs
echo "usr/share/aptsh" >> debian/dirs
echo "usr/man/man1" >> debian/dirs

rm debian/README.Debian

dpkg-buildpackage -rfakeroot

cd ..

for i in aptsh*
do 
	z=`echo $i | perl -p -e 's/^.*deb$/1/;if ($_ != "1"){$_ = "0\n"}'`
	if [ $z = "0" ]; then 
		echo Removing $i
		rm -rf $i
	fi
done

