include $(top_srcdir)/config/subdir.am

SUBDIRS = database

cfgfiles_IN = \
    authproxy-config.in \
    authrules.xml.in \
    callresolver-config.in \
    proxy-config.in \
    forwardingrules.xml.in

EXTRA_DIST = \
    sipproxy.process.xml.in \
    sipauthproxy.process.xml.in \
    sipxcallresolver.process.xml.in \
    $(cfgfiles_IN)

PROCESS_XML_FILES = \
	$(DESTDIR)$(SIPX_PROCDIR)/sipproxy.process.xml \
	$(DESTDIR)$(SIPX_PROCDIR)/sipauthproxy.process.xml

if ENABLE_CDR
PROCESS_XML_FILES += \
	$(DESTDIR)$(SIPX_PROCDIR)/sipxcallresolver.process.xml
endif

CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file))
install-exec-hook : \
	$(CFG_FILES) \
	upgrade-add-domain \
	upgrade-auth-rules \
	upgrade-forwarding-rules \
	upgrade-auth-plugins

install-data-hook : \
	$(PROCESS_XML_FILES)

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
	$(LocalizeSipXconfig) $< > $@

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

.PHONY: upgrade-auth-plugins
upgrade-auth-plugins :
	@echo -n "        Checking for authproxy plugins..."
	@if ! grep libEnforceAuthRules.so $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo -n " 'authrules' added"; \
		( echo "" \
		; echo "SIP_AUTHPROXY_HOOK_LIBRARY.authrules : $(libdir)/sipxauthproxy/libEnforceAuthRules.so" \
		; echo "SIP_AUTHPROXY.authrules.RULES        : $(sysconfdir)/sipxpbx/authrules.xml" \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in; \
	else \
		echo -n " 'authrules' ok"; \
	fi; \
	if ! grep libCallerAlias.so $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo -n " 'fromalias' added"; \
		( echo "" \
		; echo "SIP_AUTHPROXY_HOOK_LIBRARY.fromalias : $(libdir)/sipxauthproxy/libCallerAlias.so" \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in; \
	else \
		echo -n " 'fromalias' ok"; \
	fi
	@echo ""


.PHONY: upgrade-add-domain
upgrade-add-domain :
	@echo -n "        Checking for domain configuration..."
	@if ! grep -e '^SIP_AUTHPROXY_DOMAIN_NAME\>' $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in > /dev/null 2>&1 ; \
	then \
		echo " adding "; \
		( echo "" \
		; echo 'SIP_AUTHPROXY_DOMAIN_NAME : $${SIPXCHANGE_DOMAIN_NAME}' \
		) >> $(DESTDIR)$(sysconfdir)/sipxpbx/authproxy-config.in; \
	else \
		echo " ok"; \
	fi

