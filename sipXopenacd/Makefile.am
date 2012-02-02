include config/utility.am
include config/project.am

SUBDIRS = \
	src \
	bin \
	etc

@PACKAGE@_EXTRA_DIST = \
	README \
	LICENSE

install-exec-hook:
	$(INSTALL) -d -m 755 $(DESTDIR)@SIPX_LOGDIR@/openacd $(DESTDIR)@SIPX_VARDIR@/openacd