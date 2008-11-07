AM_CFLAGS = -DDATE_BINARY=\"@DATE_BINARY@\"

libexec_sipxecsdir = @SIPX_LIBEXECDIR@
libexec_sipxecs_PROGRAMS = sipx-sudo-date

sipx_sudo_date_SOURCES = \
	$(top_builddir)/src/sipxchangedate/sipx-change-date.c

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-date
	chmod 4750 $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-date


