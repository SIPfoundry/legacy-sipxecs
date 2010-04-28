include $(top_srcdir)/config/subdir.am

CLEANFILES=

include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
  Doxyfile.project.in \
  footer.html \
  credential.xml \
  permission.xml \
  INSTALL.ssl.in

if GENERATE_MANPAGES
MAN_PAGES_OPT = gen-ssl-keys.1
endif

man_MANS = $(MAN_PAGES_OPT)

gen-ssl-keys.xml : INSTALL.ssl.in
	$(ASCIIDOC) -d manpage -b docbook -o $@ $<

%.1 : %.xml
	$(XSLTPROC) $(DOCBOOK_2_MAN_XSL) $<
