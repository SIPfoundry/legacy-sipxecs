## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am

include ant-targets.am

if BUILD_OPENFIRE

BUILD_ANT=all-ant
CHECK_ANT=check-ant
INSTALL_ANT=install-ant

bin_SCRIPTS = sipxopenfire.sh sipxopenfire-initdb.sql sipxopenfire-setup.sh

RPMS=$(RPM)

else

BUILD_ANT=
CHECK_ANT=
INSTALL_ANT=

endif

all : $(BUILD_ANT)

install : $(INSTALL_ANT)

check : $(CHECK_ANT)

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxopenfire.spec.in \
  sipxopenfire.spec \
  sipxopenfire.sh.in \
  sipxopenfire-process.xml.in \
  sipxopenfire-initdb.sql \
  sipxopenfire-database.xml \
  sipxopenfire-setup.sh.in \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  presence-plugin/plugin.xml \
  presence-plugin/web/WEB-INF/web.xml \
  openfire.xml \
  web.xml.enable_openfire_admin_console \
  web.xml.disable_openfire_admin_console \
  etc/sipxopenfire-prompts.properties \
  src

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

SIPX_PROCESS_DEFS = \
	sipxopenfire 

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxopenfire-setup.sh : sipxopenfire-setup.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxopenfire.sh : sipxopenfire.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxopenfire.spec : sipxopenfire.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	
DISTCLEANFILES = config.properties sipxopenfire-setup.sh sipxopenfire.sh sipxopenfire.spec
