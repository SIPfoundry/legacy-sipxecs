## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

CGIS = \
	configuration.cgi.pre \
	dialog-event.cgi.pre \
	index.html.pre \
	registrations.cgi.pre \
	registrations_history.cgi.pre \
	trace.cgi.pre

IMAGES = \
	 Nortel.gif \
	 sipviewer-install.jar \
	 sipXecs.png

EXTRA_DIST = \
	$(foreach cgi,$(CGIS),$(cgi).in) \
	$(foreach img,$(IMAGES),$(img))

install-exec-hook: $(foreach cgi,$(CGIS),$(DESTDIR)@SIPX_INTEROP@/http_rootdir/$(cgi))

install-data-hook: $(foreach img,$(IMAGES),$(DESTDIR)@SIPX_INTEROP@/http_rootdir/$(img))

$(foreach cgi,$(CGIS),$(cgi)) : % : $(srcdir)/%.in 
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/sipviewer-install.jar : sipviewer-install.jar $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/configuration.cgi.pre : configuration.cgi.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/dialog-event.cgi.pre : dialog-event.cgi.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/registrations.cgi.pre : registrations.cgi.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/registrations_history.cgi.pre : registrations_history.cgi.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/trace.cgi.pre : trace.cgi.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/index.html.pre : index.html.pre $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/Nortel.gif : $(srcdir)/Nortel.gif $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir/sipXecs.png : $(srcdir)/sipXecs.png $(DESTDIR)@SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

$(DESTDIR)@SIPX_INTEROP@/http_rootdir : $(DESTDIR)@SIPX_INTEROP@
	$(INSTALL) -d $@

$(DESTDIR)@SIPX_INTEROP@ :
	$(INSTALL) -d $@
