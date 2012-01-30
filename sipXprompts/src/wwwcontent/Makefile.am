
aapromptsdir=@SIPX_VXMLDATADIR@/prompts
dist_aaprompts_DATA = \
	stdprompts_en/autoattendant.wav \
	stdprompts_en/afterhours.wav

promptsdir = @wwwdir@/doc/stdprompts_en
dist_prompts_DATA = \
	$(shell cd $(srcdir); find stdprompts_en -maxdepth 1 -name '*.wav')

confpromptsdir = @wwwdir@/doc/stdprompts_en/conf
dist_confprompts_DATA = \
	$(shell cd $(srcdir); find stdprompts_en/conf -maxdepth 1 -name '*.wav')

musicdir = @wwwdir@/doc/stdprompts_en/music/8000
dist_music_DATA = \
	$(shell cd $(srcdir); find stdprompts_en/music/8000 -maxdepth 1 -name '*.wav')

install-data-hook:
	cd $(DESTDIR)@wwwdir@/doc; \
	  [ -L stdprompts ] || ln -s stdprompts_en stdprompts
