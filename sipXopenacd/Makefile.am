include $(top_srcdir)/config/project.am

RPMS =  $(RPM)
#	$(RPM)  \
#	$(DEST_RPM)/@PACKAGE@-$(SIPX_VERSION).@RPM_TARGET_ARCH@.rpm

SIPX_PROCESS_DEFS = \
	openacd


SUBDIRS = \
	bin

DISTCLEANFILES = openacd-process.xml

EXTRA_DIST = \
	README \
	CONTRIBUTORS \
	BUILDSTAMP \
	SVN-VERSION \
	LICENSE	\
	config/sipX-config.in \
	config/sipX-buildstamp.cpp.in \
	config/sipX-buildstamp.h.in \
	config/svn-version \
	sipxopenacd.spec.in \
	sipxopenacd.spec \
	openacd-process.xml.in \
	openacd-process.xml \
	bin/openacd_setup.sh.in \
	openacd_setup.sh	\
	bin/openacd.sh.in \
	openacd.sh

sipxopenacd.spec : sipxopenacd.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxopenacd-process : sipxopenacd-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

openacd_setup.sh : bin/openacd_setup.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

openacd.sh : bin/openacd.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

