include $(top_srcdir)/config/subdir.am

CLEANFILES=

include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
  Doxyfile.project.in \
  footer.html \
  credential.xml \
  permission.xml

if GENERATE_DOCBOOK

EXTRA_DIST += \
	INSTALL.ssl.in \
	$(top_srcdir)/config/sipxecs_entities.ent

DOCBOOK_XML_DOCS=INSTALL.ssl

include $(top_srcdir)/config/docbook.am

if GENERATE_DOCBOOK_HTML

EXTRA_DIST+= \
	$(top_srcdir)/config/sipXecs.css

DOCBOOK_OUTPUTS+=sipXecs.css

CLEANFILES+=sipXecs.css

endif # GENERATE_DOCBOOK_HTML

endif # GENERATE_DOCBOOK

install-data-hook: $(foreach doc,$(DOCBOOK_OUTPUTS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@;
