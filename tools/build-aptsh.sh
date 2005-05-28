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
elif [ $1 = 'release' ]; then
	svn checkout svn://svn.berlios.de/aptsh/trunk/
	mv trunk/* .
	rm -rf trunk
	f=aptsh-$2
else
	f=$1
fi

mkdir $f

cp -rf * $f/

cd $f

yes "" | dh_make -s -e wrochniak@gmail.com

copy1=$(head -n 1 debian/copyright)
copy2=$(head -n 2 debian/copyright | awk 'BEGIN {z=0} //{z++; if (z == 2) {print $_}}')

echo $copy1 > debian/copyright
echo $copy2 >> debian/copyright
cat << EOF >> debian/copyright

It was downloaded from http://aptsh.berlios.de

Copyright: Marcin Wrochniak

Upstream Author: Marcin Wrochniak <wrochniak@gmail.com>

License:

Aptsh is licened under the terms of the GNU General Public License (GPL),
version 2.0 or later, as published by the Free Software Foundation.  See
the file, /usr/share/common-licenses/GPL, or
<http://www.gnu.org/copyleft/gpl.txt> for the terms of the latest version
of the GNU General Public License.
EOF

perl -pi -e 's/^Section: unknown$/Section: admin/g' debian/control
perl -pi -e 's/^Maintainer.*$/Maintainer: Marcin Wrochniak <wrochniak\@gmail\.com>/g' debian/control
perl -pi -e 's/^Description.*$/Description: APT interactive shell/g' debian/control
cat debian/control | head -n $[`wc -l debian/control | awk '{print $1}'`-1] > control_tmp
cp control_tmp debian/control
echo " Aptsh helps in managing packages by providing very nice pseudo-shell." >> debian/control
rm control_tmp

#echo "/etc/aptsh.conf" > debian/conffiles - debhelper does this automatically

echo "etc"  >> debian/dirs
echo "usr/share/aptsh" >> debian/dirs
echo "usr/share/man/man1" >> debian/dirs

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

