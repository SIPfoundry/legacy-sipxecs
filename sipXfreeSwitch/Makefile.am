include config/project.am

SIPX_PROCESS_DEFS = \
	freeswitch

SUBDIRS = \
	bin

DISTCLEANFILES = freeswitch-process.xml

@PACKAGE@_EXTRA_DIST = \
	README \
	LICENSE	\
	freeswitch-process.xml.in \
	freeswitch-process.xml \
	bin/freeswitch_setup.sh.in \
	freeswitch_setup.sh	\
	bin/freeswitch.sh.in \
	freeswitch.sh	

sipxfreeswitch-process : sipxfreeswitch-process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

freeswitch_setup.sh : bin/freeswitch_setup.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

freeswitch.sh : bin/freeswitch.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

