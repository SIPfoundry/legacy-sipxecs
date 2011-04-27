include $(top_srcdir)/config/subdir.am
include $(top_srcdir)/config/utility.am

all : $(conf_FILES)

EXTRA_DIST = \
	$(conf_SOURCE) \
	$(tftp_DATA) \
	$(audiocodes_DATA)

tftpdir = @SIPX_VARDIR@/configserver/phone/profile/tftproot
tftp_DATA = \
	ciscoIp/syncinfo.xml \
	ciscoAta/sipx7912.logo \
	preflight/00D01EFFFFFE

httpdir = @SIPX_VARDIR@/configserver/phone/profile/docroot
http_DATA = \
	preflight/00D01EFFFFFE

audiocodesdir = @SIPX_VARDIR@/audiocodes
audiocodes_DATA = \
	audiocodes/MP11x-02-1-FXS_16KHZ.dat \
	audiocodes/usa_tones_12.dat

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
	$(shell find $(srcdir) -type f -not \( \
	  -name 'Makefile*' \
	  -o -path '*/out/*' \
	  $(foreach F,$(tftp_DATA),-o -path '*/$(F)') \
	  $(foreach F,$(audiocodes_DATA),-o -path '*/$(F)') \
	  \) -printf '%P\n') \
	VM_global_library.vm \
	sipxconfig.properties \
	sipxconfig.properties.map \
	setting.dtd \
	setting.xsl

conf_FILES = $(foreach F,$(conf_SOURCE),out/$F)
out/% : % Makefile
	test -d $(@D) || mkdir -p $(@D)
	$(info $< ->  $@)
	@$(call SearchAndReplace, $<,  $@, \
	  YUM_EXISTS \
	  MRTG_ENABLED \
	  JAVAC_DEBUG \
	  POSTGRESQL_USER \
	\)

$(emptydirs) :
	mkdir -p $@

install-data-hook : $(conf_FILES) $(emptydirs)
	echo "$$Installer" | \
	  InstallFileList="$(conf_SOURCE)" \
	  InstallSourceDir=out \
	  InstallDestDir=$(DESTDIR)@SIPX_CONFDIR@ \
	  bash
