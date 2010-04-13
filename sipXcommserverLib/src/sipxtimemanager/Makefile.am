AM_CFLAGS = -DSIPX_BINDIR=\"@SIPX_BINDIR@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipx-sudo-time-manager

sipx_sudo_time_manager_SOURCES = \
	$(top_builddir)/src/sipxtimemanager/sipx-time-manager.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-time-manager
	chmod 4750 $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-time-manager


