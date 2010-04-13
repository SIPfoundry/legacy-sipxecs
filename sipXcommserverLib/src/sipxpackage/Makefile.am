AM_CFLAGS = -DSIPX_BINDIR=\"@SIPX_BINDIR@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipxpackage

sipxpackage_SOURCES = \
	$(top_builddir)/src/sipxpackage/sipxpackage.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipxpackage
	chmod 4750 $(DESTDIR)$(libexecdir)/sipXecs/sipxpackage
