include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
  Doxyfile.in \
  header.html \
  footer.html

all: doc

doc: @PACKAGE@

@PACKAGE@: Doxyfile
	rm -rf @PACKAGE@
	@DOXYGEN@

Doxyfile: $(srcdir)/Doxyfile.in
	$(LocalizeSipXconfig) \
		-e 's,@enable_html_docs\@,@enable_html_docs@,g' \
		-e 's,@enable_latex_docs\@,@enable_latex_docs@,g' \
		-e 's,@enable_dot\@,@enable_dot@,g' \
		-e 's,@srcdir\@,$(srcdir),g' \
		-e 's,@top_srcdir\@,$(top_srcdir),g' \
	$(srcdir)/Doxyfile.in > Doxyfile

install: install-doc

.PHONY: install-doc
install-doc: @PACKAGE@
	rm -rf $(DESTDIR)@SIPX_DOCDIR@/@PACKAGE@
	@INSTALL@ -d $(DESTDIR)@SIPX_DOCDIR@/@PACKAGE@
	find @PACKAGE@ -type f -print \
	| xargs --replace=% @INSTALL@ -m u=rw,go=r % $(DESTDIR)@SIPX_DOCDIR@/%

clean: clean-doc

.PHONY: clean-doc
clean-doc:
	rm -rf @PACKAGE@ Doxyfile
