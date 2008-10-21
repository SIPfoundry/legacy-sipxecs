include $(top_srcdir)/config/subdir.am

cfgfiles_IN = \
    freeswitch_config.defs

EXTRA_DIST = \
    freeswitch-process.xml.in \
    $(cfgfiles_IN)

### if ENABLE_CONFERENCE
SIPX_PROCESS_DEFS = \
	freeswitch
### endif

CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file))
install-exec-hook : \
	$(CFG_FILES)

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
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

DISTCLEANFILES = freeswitch_config.defs.local
