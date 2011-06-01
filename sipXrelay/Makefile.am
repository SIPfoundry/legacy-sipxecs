include config/utility.am
include config/project.am
include config/utility.am
include common.am

SUBDIRS = \
	src \
	.

@PACKAGE@_EXTRA_DIST = \
	$(proc_DATA:=.in) \
	$(bin_SCRIPTS:=.in)

bin_SCRIPTS = \
	sipxrelay.sh

procdir = \
	$(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxrelay-process.xml

$(proc_DATA) $(bin_SCRIPTS) : % : %.in Makefile
	$(call SearchAndReplace,$<,$@,relay_PKGS)

CLEANFILES = $(proc_DATA) $(bin_SCRIPTS)
