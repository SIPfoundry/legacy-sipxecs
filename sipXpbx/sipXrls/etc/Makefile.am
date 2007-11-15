include $(top_srcdir)/config/subdir.am

ETC_FILES =

ETC_LOCALIZE_FILES = \
    sipxrls-config.in.in

EXTRA_DIST = \
    $(ETC_FILES) \
    sipxrls.process.xml.in

install-data-hook : $(DESTDIR)$(SIPX_PROCDIR)/sipxrls.process.xml	
	@for f in $(ETC_LOCALIZE_FILES); \
	do \
		$(LocalizeSipXconfig) $(srcdir)/$$f > $${f%.in} ; \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$${f%.in}; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$${f%.in}"; \
			$(INSTALL) -D -m 644 $${f%.in} $(DESTDIR)$(sysconfdir)/sipxpbx/$${f%.in}; \
		else \
			diff $(srcdir)/$${f%.in} $(DESTDIR)$(sysconfdir)/sipxpbx/$${f%.in} > /dev/null 2>&1 \
			|| (  echo "Using existing $(srcdir)/$${f%.in} - new file is $(DESTDIR)$(sysconfdir)/sipxpbx/$${f%.in}.NEW" \
				; $(INSTALL) -D -m 644 $(srcdir)/$${f%.in} $(DESTDIR)$(sysconfdir)/sipxpbx/$${f%.in}.NEW; \
				); \
	    fi; \
	done
	@for f in $(ETC_FILES); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
			diff $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f > /dev/null 2>&1 \
			|| (  echo "Using existing $(srcdir)/$$f - new file is $(DESTDIR)$(sysconfdir)/sipxpbx/$$f.NEW" \
				; $(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f.NEW; \
				); \
	    fi; \
	done
