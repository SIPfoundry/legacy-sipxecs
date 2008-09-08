include $(top_srcdir)/config/subdir.am

SUBDIRS = database

cfgfiles_IN = \
	authrules.xml.in \
	sipxcallresolver-agent-config \
	forwardingrules.xml.in

process_xml_files_IN = \
	sipXproxy.process.xml.in \
	sipxcallresolver-agent.process.xml.in \
	sipxcallresolver.process.xml.in

EXTRA_DIST = \
	sipXproxy-process.xml.in \
	sipxcallresolver-agent-process.xml.in \
	sipxcallresolver-process.xml.in \
	$(process_xml_files_IN) \
	$(cfgfiles_IN)

SIPX_PROCESS_DEFS=\
	sipXproxy \
	sipxcallresolver-agent \
	sipxcallresolver

PROC_FILES = $(foreach file,$(process_xml_files_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/process.d/$(basename $(file)))

CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file))

install-data-hook : \
	$(CFG_FILES) \
	$(PROC_FILES) \
	upgrade-auth-rules \
	upgrade-forwarding-rules

$(foreach file,$(process_xml_files_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/process.d/$(basename $(file))) : $(DESTDIR)/$(sysconfdir)/sipxpbx/process.d/% : %
	@if ! test -f $@; \
	then \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< $@ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
		    ; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi

$(foreach file,$(process_xml_files_IN),$(basename $(file))) : % : %.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) \
		$< > $@

$(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/% : %.local
	@if ! test -f $@; \
	then \
		echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< $@ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
		    ; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi

$(foreach file,$(cfgfiles_IN),$(file).local) : %.local : %
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

.PHONY: upgrade-auth-rules
upgrade-auth-rules:
	@echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/authrules.xml.in"
	@perl -pi \
	-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00">|' \
	$(DESTDIR)$(sysconfdir)/sipxpbx/authrules.xml.in

.PHONY: upgrade-forwarding-rules
upgrade-forwarding-rules:
	@echo "        Checking/updating schema for $(sysconfdir)/sipxpbx/forwardingrules.xml.in"
	@perl -pi \
	-e 's|<routes>|<routes xmlns="http://www.sipfoundry.org/sipX/schema/xml/forwardingrules-00-00">|' \
	$(DESTDIR)$(sysconfdir)/sipxpbx/forwardingrules.xml.in


