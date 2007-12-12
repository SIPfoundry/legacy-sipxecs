include $(top_srcdir)/config/subdir.am

EXTRA_DIST = \
  Doxyfile.in \
  footer.html \
  credential.xml \
  permission.xml \
  INSTALL.ssl.in

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

install: install-doc install-ssl-doc

.PHONY: install-doc install-ssl-doc

install-doc: @PACKAGE@
	rm -rf $(DESTDIR)@SIPX_DOCDIR@/@PACKAGE@
	@INSTALL@ -d $(DESTDIR)@SIPX_DOCDIR@/@PACKAGE@
	find @PACKAGE@ -type f -print \
	| xargs --replace=% @INSTALL@ -m u=rw,go=r % $(DESTDIR)@SIPX_DOCDIR@/%

install-ssl-doc: $(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl

$(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl: INSTALL.ssl
	@INSTALL@ -m u=rw,go=r INSTALL.ssl $(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl

INSTALL.ssl: $(srcdir)/INSTALL.ssl.in
	$(LocalizeSipXconfig) $(srcdir)/INSTALL.ssl.in > INSTALL.ssl

clean: clean-doc

.PHONY: clean-doc
clean-doc:
	rm -rf @PACKAGE@ Doxyfile
	rm -f INSTALL.ssl
