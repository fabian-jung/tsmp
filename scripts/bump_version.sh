#!/bin/bash
# This script is intended to be used by the CI to bump the version. There is no need
# to run this script manually. It will bump the version in the CMakeLists.txt, vcpkg.json
# and the arch pkgbuild. It will also create a new changelog entry and append it to the
# Changelog.md file. The script will ask for the changelog entry. The script will not
# commit or push the changes.
# The first argument is the new version number in the format major.minor.patch.

if [ "$#" -lt 1 ]; then
    echo "Illegal number of parameters"
    echo "Usage: ./bump_version <version>"
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
sed -z -i 's/\\n/\n/g' doc/Changelog.md
echo -e "" >> doc/Changelog.md
tail -n +2 doc/Changelog.md.bak >> doc/Changelog.md
rm doc/Changelog.md.bak