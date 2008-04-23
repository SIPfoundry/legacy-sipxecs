include $(top_srcdir)/config/subdir.am
include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
  Doxyfile.project.in \
  footer.html \
  credential.xml \
  permission.xml

if DOC

EXTRA_DIST += \
	INSTALL.ssl.in \
	$(top_srcdir)/config/sipxecs_entities.ent

CLEANFILES=

DOCBOOK_XML_DOCS=INSTALL.ssl

include $(top_srcdir)/config/docbook.am

endif # DOC

install-data-hook: $(foreach doc,$(DOCBOOK_OUTPUTS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@;


