include $(top_srcdir)/config/subdir.am
include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
  Doxyfile.project.in \
  footer.html \
  credential.xml \
  permission.xml \
  INSTALL.ssl.in

install: install-ssl-doc

.PHONY: install-ssl-doc
install-ssl-doc: $(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl

$(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl: INSTALL.ssl
	@INSTALL@ -m u=rw,go=r INSTALL.ssl $(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl

INSTALL.ssl: $(srcdir)/INSTALL.ssl.in
	$(LocalizeSipXconfig) $(srcdir)/INSTALL.ssl.in > INSTALL.ssl

clean: clean-ssl-doc

.PHONY: clean-ssl-doc
clean-ssl-doc:
	rm -f INSTALL.ssl
