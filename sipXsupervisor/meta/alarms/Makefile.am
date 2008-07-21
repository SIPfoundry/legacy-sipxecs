include $(top_srcdir)/config/subdir.am

alarmdir = @SIPX_DATADIR@/alarms

EXTRA_DATA = \
   alarm-strings.xml \
   sipXalarms-strings.xml

EXTRA_DIST = $(EXTRA_DATA)

install-data-hook : $(foreach alarmfile,$(schemas),$(DESTDIR)@SIPX_DATADIR@/alarms/$(alarmfile))

$(foreach alarmfile,$(alarms),$(DESTDIR)$(alarmdir)/$(alarmfile)): $(DESTDIR)$(alarmdir)/% : %
	$(INSTALL) -D -m 644 $< $@


