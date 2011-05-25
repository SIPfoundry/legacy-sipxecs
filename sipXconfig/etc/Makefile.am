include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(conf_SOURCE) \
	$(proc_DATA:=.in)

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = \
	sipxconfig-process.xml \
	tunnel-process.xml \
	sipXmrtg-process.xml

$(proc_DATA) : % : %.in
	@$(call SearchAndReplace, $<, $@)

# Directories should be created in the application layer to make it more resiliant to IT administation errors
# but these may exist for historical or special reasons
emptydirs = \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/dialplan \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/sip_profiles \
	$(DESTDIR)@SIPX_VARDIR@/mrtg/thresh \
	$(DESTDIR)@SIPX_VARDIR@/configserver/web-cert \
	$(DESTDIR)@SIPX_LOGDIR@ \
	$(DESTDIR)@SIPX_DATADIR@/devicefiles \
	$(DESTDIR)@SIPX_VARDIR@/phonebook

# automatically selects configuration files, but as importantly, also has to distinguish what is NOT
# a configuration file in the 'find' statement
conf_SOURCE = \
	$(shell cd $(srcdir)/sipxpbx; find -type f -printf '%P\n')

conf_FILES = $(addprefix out/,$(conf_SOURCE))
out/% : sipxpbx/% Makefile
	@test -d $(@D) || mkdir -p $(@D)
	@$(call SearchAndReplace, $<,  $@, \
	  YUM_EXISTS \
	  MRTG_ENABLED \
	  JAVAC_DEBUG \
	  POSTGRESQL_USER \
	\)

$(emptydirs) :
	mkdir -p $@

install-data-hook : $(conf_FILES) $(emptydirs)
	@echo "$$Installer" | \
	  InstallFileList="$(conf_SOURCE)" \
	  InstallSourceDir=out \
	  InstallDestDir=$(DESTDIR)@SIPX_CONFDIR@ \
	  bash

clean-local :
	! test -d out || rm -rf out

CLEANFILES = $(proc_DATA)
