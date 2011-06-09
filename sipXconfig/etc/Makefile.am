include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(addprefix sipxpbx/,$(conf_SOURCE)) \
	$(proc_DATA:=.in) \
	$(conf_DATA:=.in)

procdir = $(SIPX_DATADIR)/process.d
proc_DATA = \
	sipxconfig-process.xml \
	tunnel-process.xml \
	sipXmrtg-process.xml

# Directories should be created in the application layer to make it more resiliant to IT administation errors
# but these may exist for historical or special reasons
emptydirs = \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/dialplan \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/sip_profiles \
	$(DESTDIR)@SIPX_VARDIR@/mrtg/thresh \
	$(DESTDIR)@SIPX_VARDIR@/configserver/web-cert \
	$(DESTDIR)@SIPX_VARDIR@/configserver/phone/profile/tftproot \
	$(DESTDIR)@SIPX_VARDIR@/configserver/phone/profile/docroot \
	$(DESTDIR)@SIPX_LOGDIR@ \
	$(DESTDIR)@SIPX_DATADIR@/devicefiles \
	$(DESTDIR)@SIPX_VARDIR@/phonebook \
	$(DESTDIR)@SIPX_JAVADIR@/sipXconfig/plugins

confdir = @SIPX_CONFDIR@
conf_DATA = sipxconfig.properties

# automatically selects configuration files, but as importantly, also has to distinguish what is NOT
# a configuration file in the 'find' statement
conf_SOURCE = \
	$(shell cd $(srcdir)/sipxpbx; find -type f -printf '%P\n')

$(conf_DATA) $(proc_DATA) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))


out/% : sipxpbx/% Makefile
	@test -d $(@D) || mkdir -p $(@D)
	@$(call SearchAndReplace,$<,$@,$(EXTRA_REPLACE))

$(emptydirs) :
	$(INSTALL) -d $@

install-data-hook : $(addprefix out/,$(conf_SOURCE)) $(emptydirs)
	@echo "$$Installer" | \
	  InstallFileList="$(conf_SOURCE)" \
	  InstallSourceDir=out \
	  InstallDestDir=$(DESTDIR)@SIPX_CONFDIR@ \
	  bash

clean-local :
	! test -d out || rm -rf out

CLEANFILES = $(proc_DATA) $(conf_DATA)
