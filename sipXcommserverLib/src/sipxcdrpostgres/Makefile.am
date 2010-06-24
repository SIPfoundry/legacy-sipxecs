AM_CFLAGS = -DSIPX_BINDIR=\"@SIPX_BINDIR@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipxcdrpostgres

sipxcdrpostgres_SOURCES = \
	$(top_builddir)/src/sipxcdrpostgres/sipxcdrpostgres.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipxcdrpostgres
	chmod 4755 $(DESTDIR)$(libexecdir)/sipXecs/sipxcdrpostgres
