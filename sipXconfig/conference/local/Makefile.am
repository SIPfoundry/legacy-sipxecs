include $(top_srcdir)/config/subdir.am

cfgfiles_IN = \
    freeswitch.xml.in \
    modules.conf.xml.in \
    sofia.conf.xml.in \
    xml_rpc.conf.xml.in

EXTRA_DIST = \
    $(cfgfiles_IN)

###CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)/$(sysconfdir)/sipxpbx/$(file))
CFG_FILES = $(foreach file,$(cfgfiles_IN),$(DESTDIR)@FREESWITCH_PREFIX@/conf/$(file))

install-exec-hook : \
	$(CFG_FILES)

$(foreach file,$(cfgfiles_IN),$(DESTDIR)@FREESWITCH_PREFIX@/conf/$(file)) : $(DESTDIR)@FREESWITCH_PREFIX@/conf/% : %.local
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
	$(LocalizeSipXconfig) $< > $@

