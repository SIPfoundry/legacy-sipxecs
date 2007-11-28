include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
   sipxrls-config.in.in \
	sipxrls.process.xml.in

install-data-hook : \
	$(DESTDIR)$(SIPX_PROCDIR)/sipxrls.process.xml \
	$(DESTDIR)$(SIPX_CONFDIR)/sipxrls-config.in	

sipxrls.process.xml.local : $(srcdir)/sipxrls.process.xml.in
	$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(SIPX_PROCDIR)/sipxrls.process.xml: sipxrls.process.xml.local
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

sipxrls-config.in.local : $(srcdir)/sipxrls-config.in.in
	$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(SIPX_CONFDIR)/sipxrls-config.in : sipxrls-config.in.local
	@if ! test -f $@; \
	then \
	  	echo "Installing default $@"; \
		$(INSTALL) -D -m 644 $< $@; \
	else \
		diff $< @ > /dev/null 2>&1 \
		|| (  echo "Using existing $@ - new file is $@.NEW" \
			; $(INSTALL) -D -m 644 $< $@.NEW; \
			); \
	fi


