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
	    	echo "Using existing $(sysconfdir)/sipxpbx/$${f%.in}"; \
	    fi; \
	done
	@for f in $(ETC_FILES); \
	do \
		if ! test -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		then \
	    	echo "Installing default $(sysconfdir)/sipxpbx/$$f"; \
			$(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(sysconfdir)/sipxpbx/$$f; \
		else \
	    	echo "Using existing $(sysconfdir)/sipxpbx/$$f"; \
	    fi; \
	done
