include $(top_srcdir)/config/subdir.am
include $(top_srcdir)/config/doxygen.am

EXTRA_DIST = \
  Doxyfile.project.in \
  footer.html \
  credential.xml \
  permission.xml \
  INSTALL.ssl.in

$(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl: INSTALL.ssl $(DESTDIR)@SIPX_DOCDIR@
	@INSTALL@ -m u=rw,go=r $< $@

INSTALL.ssl: $(srcdir)/INSTALL.ssl.in
	$(LocalizeSipXconfig) $(srcdir)/INSTALL.ssl.in > INSTALL.ssl

CLEANFILES=INSTALL.ssl

INSTALL_DOC_TARGETS=$(DESTDIR)@SIPX_DOCDIR@/INSTALL.ssl


