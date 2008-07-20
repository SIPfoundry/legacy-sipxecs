include $(top_srcdir)/config/subdir.am

schemas = \
	sipXecs-process.xsd \
	watchdog.xsd \
	watchdogprocess.xsd \
	processdefinitions.xsd \
   alarm.xsd

alarmdir = @SIPX_DATADIR@/alarms
alarm_DATA = \
   alarms/alarm-strings.xml \
   alarms/sipXalarms-strings.xml

EXTRA_DATA = $(foreach xsd,$(schemas),$(xsd).in)

EXTRA_DIST = $(EXTRA_DATA) $(alarm_DATA)

install-data-hook : install-schema

.PHONY : install-schema
install-schema: $(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd))

$(foreach xsd,$(schemas),$(DESTDIR)@SIPX_DATADIR@/schema/$(xsd)): $(DESTDIR)@SIPX_DATADIR@/schema/% : %
	$(INSTALL) -D -m 644 $< $@

$(foreach xsd,$(schemas),$(xsd)): % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

DISTCLEANFILES = \
	$(schemas) $(alarm_DATA)

