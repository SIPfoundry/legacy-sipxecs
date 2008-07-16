## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

bin_SCRIPTS = sipxbridge.sh sipxrelay.sh


RPMS=$(RPM)

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxbridge.spec.in \
  sipxbridge.spec \
  sipxbridge.process.xml.in \
  sipxbridge-process.xml.in \
  sipxbridge.sh.in \
  sipxrelay.process.xml.in \
  sipxrelay-process.xml.in \
  sipxrelay.sh.in \
  CONTRIBUTORS \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  ant-targets.xml \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

install-data-hook : \
	$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxbridge.process.xml \
	$(DESTDIR)$(SIPX_PROCESSDIR)/sipxbridge-process.xml \
	$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxrelay.process.xml \
	$(DESTDIR)$(SIPX_PROCESSDIR)/sipxrelay-process.xml

$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxbridge.process.xml : sipxbridge.process.xml
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxrelay.process.xml : sipxrelay.process.xml
	$(INSTALL) -D -m 644 $< $@

sipxbridge.process.xml : sipxbridge.process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay.process.xml : sipxrelay.process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(SIPX_PROCESSDIR)/sipxbridge-process.xml : sipxbridge-process.xml
	$(INSTALL) -D -m 644 $< $@

$(DESTDIR)$(SIPX_PROCESSDIR)/sipxrelay-process.xml : sipxrelay-process.xml
	$(INSTALL) -D -m 644 $< $@

sipxbridge-process.xml : sipxbridge-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay-process.xml : sipxrelay-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxbridge.sh : sipxbridge.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxbridge.spec : sipxbridge.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay.sh : sipxrelay.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxrelay.spec : sipxrelay.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
