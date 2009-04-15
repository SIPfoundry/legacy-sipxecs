include $(top_srcdir)/config/subdir.am

CLEANFILES=

include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
	Doxyfile.project.in \
	uml.png \
	footer.html

DOCBOOK_XML_DOCS =

OTHER_DOCS =

include $(top_srcdir)/config/docbook.am

all: $(DOCBOOK_OUTPUTS) $(OTHER_DOCS)

install-data-hook: $(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc))

$(foreach doc,$(DOCBOOK_OUTPUTS) $(OTHER_DOCS),$(DESTDIR)@SIPX_DOCDIR@/$(doc)) : $(DESTDIR)@SIPX_DOCDIR@/% : %
	$(INSTALL) -D -m 644 $< $@;
