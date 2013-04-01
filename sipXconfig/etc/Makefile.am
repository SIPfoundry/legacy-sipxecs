include $(top_srcdir)/config/utility.am
include $(top_srcdir)/common.am

all : $(addprefix out/,$(conf_SOURCE))

EXTRA_DIST = \
	$(addprefix sipxpbx/,$(conf_SOURCE)) \
	sipxconfig.cfdat

# Directories should be created in the application layer to make it more resiliant to IT administation errors
# but these may exist for historical or special reasons
emptydirs = \
	$(DESTDIR)@SIPX_CFDATA@/1 \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/dialplan \
	$(DESTDIR)@SIPX_CONFDIR@/freeswitch/conf/sip_profiles \
	$(DESTDIR)@SIPX_CONFDIR@/database/plugin.d/v12 \
	$(DESTDIR)@SIPX_VARDIR@/mrtg/thresh \
	$(DESTDIR)@SIPX_VARDIR@/configserver/web-cert \
	$(DESTDIR)@SIPX_VARDIR@/configserver/phone/profile/tftproot \
	$(DESTDIR)@SIPX_VARDIR@/configserver/phone/profile/docroot \
	$(DESTDIR)@SIPX_LOGDIR@ \
	$(DESTDIR)@SIPX_DATADIR@/devicefiles \
	$(DESTDIR)@SIPX_VARDIR@/phonebook \
	$(DESTDIR)@SIPX_JAVADIR@/sipXconfig/plugins

cfinputsdir = @SIPX_CFINPUTS@/plugin.d
dist_cfinputs_DATA = \
	00_sipxconfig.cf \
	sipxconfig.cf \
	tftp.cf \
	ftp.cf

cflibdir = @SIPX_CFINPUTS@/plugin.d/lib
dist_cflib_DATA = \
	reset_cfkey.cf

# automatically selects configuration files, but as importantly, also has to distinguish what is NOT
# a configuration file in the 'find' statement
confdir = @SIPX_CONFDIR@
conf_SOURCE = \
	$(shell cd $(srcdir)/sipxpbx; find -type f -printf '%P\n')

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

check : validate-xml

setting_2_0.dtd setting_1_0.dtd : sipxpbx/setting.dtd
	cp $< $@

validate-xml : setting_2_0.dtd setting_1_0.dtd $(addprefix out/,$(SETTINGS_XML))
	xmllint --noout --path . --valid $(addprefix out/,$(SETTINGS_XML))

# easier to remove all non-settings xml files from list then to specify each settings xml file
SETTINGS_XML = \
	$(shell cd $(srcdir)/sipxpbx; \
	  find . \( -name '*.xml' \
	    -a -not -name appearance-groups.xml \
	    -a -not -name database.xml \
	    -a -not -path '*test.d/*' \
	    -a -not -path '*itspTemplates/*' \
	  \) \
	)
