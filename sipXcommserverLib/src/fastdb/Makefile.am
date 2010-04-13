## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/include/fastdb

noinst_LTLIBRARIES = libfastdb.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libfastdb_la_CXXFLAGS = \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\"

libfastdb_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age}

libfastdb_la_SOURCES =  \
    class.cpp \
    compiler.cpp \
    container.cpp \
    cursor.cpp \
    database.cpp \
    file.cpp \
    hashtab.cpp \
    localcli.cpp \
    query.cpp \
    repsock.cpp \
    rtree.cpp \
    server.cpp \
    stdtp.cpp \
    subsql.cpp \
    symtab.cpp \
    sync.cpp \
    ttree.cpp \
    unisock.cpp \
    wwwapi.cpp
