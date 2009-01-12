AM_CFLAGS = -DCP_BINARY=\"@CP_BINARY@\" -DMV_BINARY=\"@MV_BINARY@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipx-sudo-timezone

sipx_sudo_timezone_SOURCES = \
	$(top_builddir)/src/sipxchangetimezone/sipx-change-timezone.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-timezone
	chmod 4750 $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-timezone
