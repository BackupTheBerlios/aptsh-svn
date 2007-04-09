#!/bin/bash

# Usage: ./releaser.sh version

mkdir $1
cd $1

#svn checkout svn://svn.berlios.de/aptsh/trunk/
svn checkout svn+ssh://vrok@svn.berlios.de/svnroot/repos/aptsh/trunk
mv trunk aptsh-$1
find aptsh-$1/ -type d | grep \.svn$ | xargs rm -rf

cd aptsh-$1
./autogen.sh
dpkg-buildpackage -rfakeroot
