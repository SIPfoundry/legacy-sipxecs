AM_CFLAGS = -DSIPX_BINDIR=\"@SIPX_BINDIR@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipxzoneadmin

sipxzoneadmin_SOURCES = \
	$(top_builddir)/src/sipxzoneadmin/sipxzoneadmin.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipxzoneadmin
	chmod 4755 $(DESTDIR)$(libexecdir)/sipXecs/sipxzoneadmin
