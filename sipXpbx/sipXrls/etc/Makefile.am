include $(top_srcdir)/config/subdir.am

SIPX_PROCESS_DEFS = \
	sipxrls

EXTRA_DIST = \
	sipxrls-process.xml.in \
	sipxrls.process.xml.in

install-data-hook : \
	$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxrls.process.xml

sipxrls.process.xml.local : $(srcdir)/sipxrls.process.xml.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(SIPX_CONFDIR)/process.d/sipxrls.process.xml: sipxrls.process.xml.local
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

