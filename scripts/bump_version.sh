#!/bin/bash

if [ "$#" -lt 1 ]; then
    echo "Illegal number of parameters"
    exit 1
fi

# CMakeLists.txt
# Find and replace
## project(tsmp
## VERSION 1.0.4
# with
## project(tsmp
##    VERSION $1
sed -i -z -E "s/(project\(tsmp\s*VERSION\s*)([[:alnum:]]|\.)*/\1$1/" CMakeLists.txt

# vcpkg.json
# Find and replace
## "name": "tsmp",
## "version": "1.0.4",
# with
## "name": "tsmp",
## "version": "$1",
sed -i -z -E "s/(\s*\"name\": \"tsmp\",\s*\"version\"\:\s*\")[^\"]*(\"\,)/\1$1\2/" vcpkg.json

#arch pkgbuild
# Find and replace
## pkgver=1.0.4
# with
##  pkgver=$1
sed -i -E "s/(pkgver=)([[:alnum:]]|\.)*/\1$1/" scripts/package/arch/PKGBUILD


mv doc/Changelog.md doc/Changelog.md.bak
head -n 2 doc/Changelog.md.bak >> doc/Changelog.md
echo "Please enter the Changelog:"
echo -e "## $1\n" >> doc/Changelog.md
cat - >> doc/Changelog.md
echo -e "" >> doc/Changelog.md
tail -n +2 doc/Changelog.md.bak >> doc/Changelog.md
rm doc/Changelog.md.bak