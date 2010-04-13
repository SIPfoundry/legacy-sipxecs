include $(top_srcdir)/config/subdir.am

alarmdir = @SIPX_DATADIR@/alarms

EXTRA_DATA = \
   alarm-strings.xml \
   sipXalarms-strings.xml

EXTRA_DIST = $(EXTRA_DATA)

install-data-hook : $(foreach alarmfile,$(EXTRA_DATA),$(DESTDIR)@SIPX_DATADIR@/alarms/$(alarmfile))

$(foreach alarmfile,$(EXTRA_DATA),$(DESTDIR)$(alarmdir)/$(alarmfile)): $(DESTDIR)$(alarmdir)/% : %
	$(INSTALL) -D -m 644 $< $@


