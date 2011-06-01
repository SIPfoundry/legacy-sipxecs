aapromptsdir=@SIPX_VXMLDATADIR@/prompts
dist_aaprompts_DATA = \
	stdprompts_en/autoattendant.wav \
	stdprompts_en/afterhours.wav

wwwcontentdir = @wwwdir@/doc
dist_wwwcontent_DATA = \
	index.html \
	retrieve-update-log.cgi \
	retrieve-error-log.cgi

# we still need vxml? --douglas
aavxmldir = @wwwdir@/doc/aa_vxml
dist_aavxml_DATA = \
	aa_vxml/autoattendant-operator.vxml \
	aa_vxml/autoattendant-afterhour.vxml \
	aa_vxml/operator-schedule.xml \
	aa_vxml/dialbyname.vxml

# we still need this? --douglas
vxmldir = @wwwdir@/doc/vm_vxml
dist_vxml_DATA = \
	vm_vxml/activategreeting.vxml \
	vm_vxml/changepin.vxml \
	vm_vxml/error_handler.vxml \
	vm_vxml/forwardmessage.vxml \
	vm_vxml/login.vxml \
	vm_vxml/login_no_autoattendant.vxml \
	vm_vxml/mainmenu.vxml \
	vm_vxml/mainmenu_no_autoattendant.vxml \
	vm_vxml/manage_autoattendant.vxml \
	vm_vxml/manage_system_greetings.vxml \
	vm_vxml/msglisten.vxml \
	vm_vxml/recordgreeting.vxml \
	vm_vxml/replytovoicemail.vxml \
	vm_vxml/root.vxml.in \
	vm_vxml/savemessage.vxml \
	vm_vxml/savemessage_no_operator.vxml

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
	cd $(DESTDIR)$(wwwcontentdir); \
	  [ -L stdprompts] || $(LN_S) stdprompts_en stdprompts
