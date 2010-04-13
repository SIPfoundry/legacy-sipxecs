audio_path=@SIPX_VARDIR@/sipxpage/music

AUDIO_DIST = \
   attention.wav \
   fanfare.wav \
   fluffy.wav \
   notice.wav \
   nowhearthis.wav \
   reveille.wav \
   TadaTada.wav

EXTRA_DIST = \
	$(AUDIO_DIST)

install-data-hook : install-audio

.PHONY: install-audio
install-audio : $(foreach audio,$(AUDIO_DIST),$(DESTDIR)$(audio_path)/$(audio))

$(foreach audio,$(AUDIO_DIST),$(DESTDIR)$(audio_path)/$(audio)) : $(DESTDIR)$(audio_path)/% : $(srcdir)/%
	$(INSTALL) -D -m 644 $< $@
