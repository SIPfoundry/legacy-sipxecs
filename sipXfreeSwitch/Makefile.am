include $(top_srcdir)/config/project.am

SIPX_PROCESS_DEFS = \
	freeswitch


SUBDIRS = \
	bin

DISTCLEANFILES = freeswitch-process.xml

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
	sipxfreeswitch.spec.in \
	sipxfreeswitch.spec \
	freeswitch-process.xml.in \
	freeswitch-process.xml \
	bin/freeswitch_setup.sh.in \
	freeswitch_setup.sh	\
	bin/freeswitch.sh.in \
	freeswitch.sh	

sipxfreeswitch.spec : sipxfreeswitch.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxfreeswitch-process : sipxfreeswitch-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

freeswitch_setup.sh : bin/freeswitch_setup.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

freeswitch.sh : bin/freeswitch.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

