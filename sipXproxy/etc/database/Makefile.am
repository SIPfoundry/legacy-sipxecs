

include $(top_srcdir)/config/subdir.am

sql_files = \
    schema.sql \
    refer_uri.sql \
    view_cdrs.sql \
    index_on_time.sql \
    reference.sql \
    contact.sql \
    calldirection.sql \
    cdrremote.sql \
    transaction.sql

EXTRA_DIST = \
    $(sql_files)

install-exec-hook : $(foreach file,$(sql_files),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file))

$(foreach file,$(sql_files),$(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/$(file)) : $(DESTDIR)/$(sysconfdir)/sipxpbx/cdr/% : %
	$(INSTALL) -D -m 644 $< $@;
