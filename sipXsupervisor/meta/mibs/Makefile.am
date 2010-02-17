include $(top_srcdir)/config/subdir.am

mibdir = @SIPX_DATADIR@/mibs

EXTRA_DATA = \
   SIPXECS-ALARM-NOTIFICATION-MIB.mib

EXTRA_DIST = $(EXTRA_DATA)

install-data-hook : $(foreach mibfile,$(EXTRA_DATA),$(DESTDIR)@SIPX_DATADIR@/mibs/$(mibfile))

$(foreach mibfile,$(EXTRA_DATA),$(DESTDIR)$(mibdir)/$(mibfile)): $(DESTDIR)$(mibdir)/% : %
	$(INSTALL) -D -m 644 $< $@


