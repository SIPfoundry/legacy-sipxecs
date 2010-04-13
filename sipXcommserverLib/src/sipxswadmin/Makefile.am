AM_CFLAGS = -DSIPX_BINDIR=\"@SIPX_BINDIR@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipxswadmin

sipxswadmin_SOURCES = \
	$(top_builddir)/src/sipxswadmin/sipxswadmin.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipxswadmin
	chmod 4755 $(DESTDIR)$(libexecdir)/sipXecs/sipxswadmin
