AM_CFLAGS = -DDATE_BINARY=\"@DATE_BINARY@\"

libexec_PROGRAMS = sipx-sudo-date

sipx_sudo_date_SOURCES = \
	$(top_builddir)/src/sipxchangedate/sipx-change-date.c

$(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-date : sipx-sudo-date
	$(INSTALL) -D -m 755 $< $@

install-exec-hook : $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-date
	chmod 4750 $(DESTDIR)$(libexecdir)/sipXecs/sipx-sudo-date


