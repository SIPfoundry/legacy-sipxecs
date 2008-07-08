include $(top_srcdir)/config/subdir.am

audio_path=@SIPX_VARDIR@/sipxpage/music

EXTRA_DIST = \
   attention.wav \
   fanfare.wav \
   fluffy.wav \
   notice.wav \
   nowhearthis.wav \
   reveille.wav \
   TadaTada.wav

install-data-hook :
       @for f in $(EXTRA_DIST) ; \
       do \
          target=`basename $$f` ; \
          if [ -f $(DESTDIR)$(audio_path)/$$target ]; \
          then \
             echo "Using existing $(audio_path)/$$target"; \
          else \
             echo ""; echo "Installing default $(audio_path)/$$target"; \
             echo "Using: $(srcdir)/$$f"; \
             $(INSTALL) -D -m 644 $(srcdir)/$$f $(DESTDIR)$(audio_path)/$$target; \
          fi; \
       done
