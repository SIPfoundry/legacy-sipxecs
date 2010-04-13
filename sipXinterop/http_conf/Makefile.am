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

install-data-hook: $(foreach cfg,$(CFGFILES) $(PRE_CFGFILES:.pre=),$(DESTDIR)@SIPX_INTEROP@/http_conf/$(cfg))

$(DESTDIR)@SIPX_INTEROP@/http_conf/httpd.conf: httpd.conf $(DESTDIR)@SIPX_INTEROP@/http_conf
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_conf/httpd-sipxchange-common.conf.in: httpd-sipxchange-common.conf.in $(DESTDIR)@SIPX_INTEROP@/http_conf
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_conf/httpd-sipxchange-common-ssl.conf: httpd-sipxchange-common-ssl.conf $(DESTDIR)@SIPX_INTEROP@/http_conf
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_conf/mime.types: $(srcdir)/mime.types $(DESTDIR)@SIPX_INTEROP@/http_conf
	$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_conf :
	$(INSTALL) -d $@

$(foreach pre,$(PRE_CFGFILES),$(pre:.pre=)) : % : $(srcdir)/%.pre
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@


