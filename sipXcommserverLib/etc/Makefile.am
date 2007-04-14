## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
    keepalive.process.xml.in \
    config.defs.in

install-data-hook : \
		$(DESTDIR)$(SIPX_PROCDIR)/keepalive.process.xml \
		$(DESTDIR)$(sysconfdir)/sipxpbx/config.defs

$(DESTDIR)$(sysconfdir)/sipxpbx/config.defs: config.defs
	if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs ]; \
	then \
		echo "Using existing $(sysconfdir)/sipxpbx/config.defs"; \
	   $(INSTALL) -D -m 644 config.defs $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs.NEW; \
	else \
		echo ""; echo "Installing default $(sysconfdir)/sipxpbx/config.defs"; \
	   $(INSTALL) -D -m 644 config.defs $(DESTDIR)$(sysconfdir)/sipxpbx/config.defs; \
	fi

config.defs : $(srcdir)/config.defs.in 
		$(LocalizeSipXconfig) $(srcdir)/config.defs.in > config.defs;
