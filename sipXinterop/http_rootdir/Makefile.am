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

install-exec-hook: $(foreach cgi,$(CGIS),@SIPX_INTEROP@/http_rootdir/$(cgi))

install-data-hook: $(foreach img,$(IMAGES),@SIPX_INTEROP@/http_rootdir/$(img))

$(foreach cgi,$(CGIS),$(cgi)) : % : $(srcdir)/%.in 
	@echo "Localize $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@

@SIPX_INTEROP@/http_rootdir/sipviewer-install.jar : sipviewer-install.jar @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/configuration.cgi.pre : configuration.cgi.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/dialog-event.cgi.pre : dialog-event.cgi.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/registrations.cgi.pre : registrations.cgi.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/registrations_history.cgi.pre : registrations_history.cgi.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/trace.cgi.pre : trace.cgi.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/index.html.pre : index.html.pre @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/Nortel.gif : $(srcdir)/Nortel.gif @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir/sipXecs.png : $(srcdir)/sipXecs.png @SIPX_INTEROP@/http_rootdir
	@$(INSTALL) $< $@

@SIPX_INTEROP@/http_rootdir :
	$(INSTALL) -d $@
