include $(top_srcdir)/config/subdir.am

editscripts_IN = \
    sipXmrtg.sh

# Append to global search/replace with _very_ project specific search/replaces
edit = $(LocalizeSipXconfig) \
    -e 's,@MRTG_ENABLED\@,@MRTG_ENABLED@,g' \
    -e 's,@MRTG_BINARY\@,@MRTG_BINARY@,g'

EXTRA_DIST = \
   $(foreach file,$(editscripts_IN),$(file).in)

install-exec-hook: $(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file))

$(foreach file,$(editscripts_IN),$(file)) : % : %.in
	@echo "Localize $< -> $@"
	@$(edit) $< > $@

$(foreach file,$(editscripts_IN),$(DESTDIR)$(bindir)/$(file)) : $(DESTDIR)$(bindir)/% : %
	$(INSTALL) -D -m 755 $< $@

DISTCLEANFILES = sipXmrtg.sh
