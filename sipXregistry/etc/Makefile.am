## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/subdir.am

# Files to be installed if they do not already exist.
# But if they already exist, the current contents may need to be edited.
editetc_IN = \
    registrar-config.in \
    mappingrules.xml.in \
    fallbackrules.xml.in

EXTRA_DIST = \
  sipregistrar.process.xml.in \
  $(editetc_IN) 

# Install files in $(editetc_IN), or if they already exist, edit
# them to be compatible with the current code.

install-data-hook : \
	$(foreach etcfile,$(editetc_IN),$(DESTDIR)$(sysconfdir)/sipxpbx/$(etcfile)) \
	editetc \
	$(DESTDIR)$(SIPX_PROCDIR)/sipregistrar.process.xml

$(foreach etcfile,$(editetc_IN),$(etcfile).tmp) : %.tmp : %
	$(LocalizeSipXconfig) $< > $@

$(DESTDIR)$(sysconfdir)/sipxpbx/registrar-config.in: registrar-config.in.tmp
	if [ -f $@ ]; \
	then \
	  echo "    Using existing $@"; \
	  echo -n "        Checking mwi ..."; \
	  if grep 'SIP_REGISTRAR_HOOK_LIBRARY.MWI' $@ > /dev/null 2>&1 ; \
	  then \
	    echo " ok" ; \
	  else \
	  	 echo " adding new mwi hook"; \
	  	 echo "SIP_REGISTRAR_HOOK_LIBRARY.MWI : @SIPX_LIBDIR@/libRegistrarImpliedMWI.so" \
	  	 >> $@ ; \
	  	 perl -pi -e 's/SIP_REGISTRAR_MWI_SUBSCRIBE/SIP_REGISTRAR.MWI.UA/' $@; \
	  fi; \
	  echo -n "        Checking proxy port ..."; \
	  if grep 'SIP_REGISTRAR_PROXY_PORT' $@ > /dev/null 2>&1 ; \
	  then \
	  	 echo " ok" ; \
	  else \
	  	 echo " adding proxy port"; \
	  	 echo 'SIP_REGISTRAR_PROXY_PORT : $${PROXY_SERVER_SIP_PORT}' \
	  	 >> $@ ; \
	  fi; \
	  echo -n "        Checking for authproxy ..."; \
	  if grep 'SIP_REGISTRAR_AUTH_PROXY' $@ > /dev/null 2>&1 ; \
	  then \
	  	 echo " ok" ; \
	  else \
	  	 echo " adding authproxy"; \
	  	 echo 'SIP_REGISTRAR_AUTH_PROXY : $${AUTH_PROXY_SERVER_SIP_SRV_OR_HOSTPORT}' \
	  	 >> $@ ; \
	  fi; \
   else \
	  echo ""; echo "Installing default $@"; \
	  $(INSTALL) -D -m 644 $< $@; \
	fi

$(DESTDIR)$(sysconfdir)/sipxpbx/fallbackrules.xml.in: fallbackrules.xml.in.tmp
	  if [ -f $@ ]; \
	  then \
		echo "    Using existing $@"; \
		echo "        Checking/updating schema for $@"; \
		perl -pi \
			-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
			$@; \
	  else \
		 echo ""; echo "Installing default $@"; \
		 $(INSTALL) -D -m 644 $< $@; \
	  fi

$(DESTDIR)$(sysconfdir)/sipxpbx/mappingrules.xml.in: mappingrules.xml.in.tmp
	  if [ -f $@ ]; \
	  then \
		echo "    Using existing $@"; \
		echo "        Checking/updating schema for $@"; \
		perl -pi \
			-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
			$@; \
	  else \
		 echo ""; echo "Installing default $@"; \
		 $(INSTALL) -D -m 644 $< $@; \
	  fi

.PHONY: editetc
editetc : $(DESTDIR)$(bindir)/registrar-config-upgrade
	@ . @SIPX_LIBEXECDIR@/sipx-utils.sh || exit 1; \
	for f in $(editetc_IN); \
	do \
	  target=`basename $$f`; \
	  $(LocalizeSipXconfig) $(srcdir)/$$f > $$target.tmp; \
	  tgtdir=`dirname $(DESTDIR)$(sysconfdir)/sipxpbx/$$target`; \
	  if [ -f $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ]; \
	  then \
		echo "    Using existing $(DESTDIR)$(sysconfdir)/sipxpbx/$$target"; \
		case $$target in \
			registrar-config.in) \
				echo -n "        Checking mwi in $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ..."; \
				if grep 'SIP_REGISTRAR_HOOK_LIBRARY.MWI' $(DESTDIR)$(sysconfdir)/sipxpbx/$$target > /dev/null; \
				then \
					echo " ok" ; \
				else \
					echo " adding"; \
					echo "SIP_REGISTRAR_HOOK_LIBRARY.MWI : @SIPX_LIBDIR@/libRegistrarImpliedMWI.so" \
					>> $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ; \
					perl -pi \
					-e 's/SIP_REGISTRAR_MWI_SUBSCRIBE/SIP_REGISTRAR.MWI.UA/' \
					$(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				fi; \
				echo -n "        Checking proxy port in $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ..."; \
				if grep 'SIP_REGISTRAR_PROXY_PORT' $(DESTDIR)$(sysconfdir)/sipxpbx/$$target > /dev/null; \
				then \
					echo " ok" ; \
				else \
					echo " adding"; \
					echo 'SIP_REGISTRAR_PROXY_PORT : $${PROXY_SERVER_SIP_PORT}' \
					>> $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ; \
				fi; \
				echo "        DESTDIR $(DESTDIR)" ; \
				echo "	      sysconfdir $(sysconfdir)" ; \
				echo "        file $(DESTDIR)$(sysconfdir)/sipxpbx/$$target" ; \
				echo -n "        Checking redirector configuration in $(DESTDIR)$(sysconfdir)/sipxpbx/$$target ..."; \
				if grep '^SIP_REDIRECT_HOOK_LIBRARY\.' $(DESTDIR)$(sysconfdir)/sipxpbx/$$target > /dev/null; \
				then \
					echo " ok" ; \
				else \
					echo " adding"; \
					$(DESTDIR)$(bindir)/registrar-config-upgrade ; \
				fi; \
				;; \
			fallbackrules.xml.in|mappingrules.xml.in) \
				echo "        Checking/updating schema for $(DESTDIR)$(sysconfdir)/sipxpbx/$$target"; \
				perl -pi \
				-e 's|<mappings>|<mappings xmlns="http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00">|' \
				$(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
				;; \
		esac; \
	  else \
		echo ""; echo "Installing default $(DESTDIR)$(sysconfdir)/sipxpbx/$$target"; \
		$(INSTALL) -D -m 644 $$target.tmp $(DESTDIR)$(sysconfdir)/sipxpbx/$$target; \
	  fi; \
	done
