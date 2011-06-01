include config/utility.am
include config/project.am
include config/utility.am
include common.am

@PACKAGE@_EXTRA_DIST = \
	README \
	$(proc_DATA:=.in) \
	$(bin_SCRIPTS:=.in) \
	LICENSE

SUBDIRS = \
	src \
	.

bin_SCRIPTS = \
	sipxrest.sh

procdir = \
	$(SIPX_DATADIR)/process.d

proc_DATA = \
	sipxrest-process.xml

$(proc_DATA) $(bin_SCRIPTS) : % : %.in
	@$(call SearchAndReplace,$<,$@,rest_PKGS)

CLEANFILES = $(proc_DATA)
