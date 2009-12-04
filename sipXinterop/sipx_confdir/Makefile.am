## Process this file with automake to produce Makefile.in
include ../config/subdir.am

include ../interop.am

CONFIGS = \
	alarm-config.xml.in \
	authrules.xml.in \
	autoattendants.xml.in \
	fallbackrules.xml.in \
	forwardingrules.xml.in \
	freeswitch/conf/freeswitch.xml.in \
	freeswitch/conf/vars.xml.in \
	freeswitch/conf/dialplan/sipX_context.xml.in \
	freeswitch/conf/sip_profiles/sipX_profile.xml.in \
	mappingrules.xml.in \
	mediaserver-config.in \
	nattraversalrules.xml.in \
	registrar-config.in \
	sipxdialog-config.in \
	sipxivr.properties.in \
	sipxpark-config.in \
	sipXproxy-config.in \
	sipxrls-config.in \
	sipxsupervisor-config.in \
	status-config.in \
	status-plugin.xml.in \
	voicemail.xml.in

EXTRA_DIST = $(foreach conf,$(CONFIGS),$(conf).pre) interopattendant.wav

install-data-hook: \
	$(foreach conf,$(CONFIGS),@SIPX_INTEROP@/conf/$(conf)) \
	@SIPX_INTEROP@/conf/interopattendant.wav

$(foreach conf,$(CONFIGS),@SIPX_INTEROP@/conf/$(conf)) : @SIPX_INTEROP@/conf/% : %
	$(INSTALL) $< $@

$(foreach conf,$(CONFIGS),$(conf)) : % : %.pre
	@test -d $$(dirname $@) || mkdir -vp $$(dirname $@) 
	@echo "Localizing $< -> $@"
	@$(INTEROP_SUBSTITUTE) $< > $@

@SIPX_INTEROP@/conf/interopattendant.wav: interopattendant.wav @SIPX_INTEROP@/conf
	$(INSTALL) $< $@

@SIPX_INTEROP@/conf/freeswitch/conf/freeswitch.xml.in : @SIPX_INTEROP@/conf/freeswitch/conf

@SIPX_INTEROP@/conf/freeswitch/conf/vars.xml.in : @SIPX_INTEROP@/conf/freeswitch/conf

@SIPX_INTEROP@/conf/freeswitch/conf/dialplan/sipX_context.xml.in : @SIPX_INTEROP@/conf/freeswitch/conf/dialplan

@SIPX_INTEROP@/conf/freeswitch/conf/sip_profiles/sipX_profile.xml.in : @SIPX_INTEROP@/conf/freeswitch/conf/sip_profiles

@SIPX_INTEROP@/conf/freeswitch/conf/sip_profiles: @SIPX_INTEROP@/conf/freeswitch/conf
	$(INSTALL) -d $@

@SIPX_INTEROP@/conf/freeswitch/conf/dialplan : @SIPX_INTEROP@/conf/freeswitch/conf
	$(INSTALL) -d $@

@SIPX_INTEROP@/conf/freeswitch/conf : @SIPX_INTEROP@/conf/freeswitch
	$(INSTALL) -d $@

@SIPX_INTEROP@/conf/freeswitch : @SIPX_INTEROP@/conf
	$(INSTALL) -d $@

@SIPX_INTEROP@/conf :
	$(INSTALL) -d $@




