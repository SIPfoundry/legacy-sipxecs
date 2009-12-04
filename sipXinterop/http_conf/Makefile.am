## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

PRE_CFGFILES = \
	 httpd.conf.pre \
	 httpd-sipxchange-common.conf.in.pre \
	 httpd-sipxchange-common-ssl.conf.pre

CFGFILES = \
	 mime.types

EXTRA_DIST = \
	 $(PRE_CFGFILES) \
	 $(CFGFILES)

install-data-hook: $(foreach cfg,$(CFGFILES) $(PRE_CFGFILES:.pre=),$(sysconfdir)/sipxpbx/$(cfg))

$(sysconfdir)/sipxpbx/httpd.conf: httpd.conf
	$(INSTALL) $< $@

$(sysconfdir)/sipxpbx/httpd-sipxchange-common.conf.in: httpd-sipxchange-common.conf.in
	$(INSTALL) $< $@

$(sysconfdir)/sipxpbx/httpd-sipxchange-common-ssl.conf: httpd-sipxchange-common-ssl.conf
	$(INSTALL) $< $@

$(sysconfdir)/sipxpbx/mime.types: $(srcdir)/mime.types
	$(INSTALL) $< $@

$(foreach pre,$(PRE_CFGFILES),$(pre:.pre=)) : % : $(srcdir)/%.pre
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@


