#! /bin/bash

# Script to check that "make clean" really does remove all build artifacts.

# Temporary directory under which the source trees will be placed.
T="${TMPDIR:-/tmp}/$$clean-check"

# The directory containing this script.
D="$( cd "$( dirname "$0" )" ; pwd )"
# The name of Makefile.devel in the same directory as this script.
MAKEFILE="$D/Makefile.devel"

# Show the user what we are doing, as some of these commands take a long time.
set -x
# Stop if there is an error.
set -e

# Make the temporary directory.
mkdir "$T"
cd "$T"

# Make a clean checkout of the source.
mkdir "$T/build"
make -f "$MAKEFILE" -C "$T/build" checkout
# Get the list of files from the fresh checkout.
( cd "$T/build/sipX" ;
  find . -name .svn -prune -false -o -type f | sort ) >"$T/clean-files"

# Make an install directory.
mkdir "$T/install"
# Make an Apache modules directory in the install directory, so the install
# process will use it instead of /lib/httpd/modules.
mkdir -p "$T/install/lib/httpd/modules"
# Build everything.
make PREFIX="$T/install" -f "$MAKEFILE" -C "$T/build/sipX" build-install
# List generated files that aren't svn:ignore.
( cd "$T/build/sipX" ; svn status |
  grep '^?      ' |
  sed -e 's/^?      //' >$T/unignored-files )
# Clean the build directory..
make PREFIX="$T/install" -f "$MAKEFILE" -C "$T/build/sipX" distclean

# Get the list of files after build and distclean.
( cd "$T/build/sipX" ;
  find . -name .svn -prune -false -o -type f | sort ) >"$T/build-files"
# Take the differences between the two lists of files.
comm -13 "$T/clean-files" "$T/build-files" >"$T/uncleaned-files"
comm -23 "$T/clean-files" "$T/build-files" >"$T/destroyed-files"

# Print the messages.
echo "----- The following files are build artifacts not recorded in svn:ignore:"
cat "$T/unignored-files"
echo "----- [end]"

echo "----- The following files are build artifacts not removed by 'make distclean':"
cat "$T/uncleaned-files"
echo "----- [end]"

echo "----- The following files are source-controlled but are removed by 'make distclean':"
cat "$T/destroyed-files"
echo "----- [end]"

# Delete the temporary directory.
#rm -rf "$T"
