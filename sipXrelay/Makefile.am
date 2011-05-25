include config/project.am
include config/utility.am

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

$(proc_DATA) $(bin_SCRIPTS) : % : %.in
	$(call SearchAndReplace,$<, $@)

CLEANFILES = $(proc_DATA) $(bin_SCRIPTS)
