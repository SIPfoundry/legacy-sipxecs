%define _prefix   /usr/local/freeswitch
%define prefix    %{_prefix}
%define fs_user   root
%define debug_package %{nil}

Name:         sipx-freeswitch
Summary:      FreeSWITCH open source telephony platform (sipX integration)
License:      MPL
Group:        Productivity/Telephony/Servers
Version:      %{VERSION}
Release:      %{RELEASE}
URL:          http://www.freeswitch.org/
Packager:     Avaya
Vendor:       http://www.voiceworks.pl/
Source0:      %{SOURCE}
Patch0:       mod_event_socket.patch
Prefix:       %{prefix}

AutoReqProv:  yes

%if 0%{?suse_version} > 100
#BuildRequires: openldap2-devel
BuildRequires: lzo-devel
%else
BuildRequires: openldap-devel
%endif
BuildRequires: autoconf
BuildRequires: automake
BuildRequires: curl-devel
BuildRequires: gcc-c++

%if 0%{?suse_version} >= 1100
BuildRequires: libgnutls-devel
%else
BuildRequires: gnutls-devel
%endif
#BuildRequires: libtool >= 1.5.14
BuildRequires: ncurses-devel
BuildRequires: openssl-devel
BuildRequires: perl
%if 0%{?fedora} >= 8
BuildRequires: perl-devel
BuildRequires: perl-ExtUtils-Embed
%endif
BuildRequires: pkgconfig
#BuildRequires: termcap
BuildRequires: unixODBC-devel
BuildRequires: gdbm-devel
%if 0%{?suse_version} > 100
BuildRequires: db43-devel
%else
BuildRequires: db4-devel
%endif

%if %{?suse_version:1}0
%if 0%{?suse_version} > 910
#BuildRequires: autogen
%endif
%endif

# Fedora doesn't seem to have 'which' as part of the base system
%if %{?fedora:1}0
BuildRequires: which
%endif

%if 0%{?suse_version} > 800
#PreReq:       /usr/sbin/useradd /usr/sbin/groupadd
PreReq:       %insserv_prereq %fillup_prereq
%endif

BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
FreeSWITCH is an open source telephony platform designed to facilitate the creation of voice 
and chat driven products scaling from a soft-phone up to a soft-switch.  It can be used as a 
simple switching engine, a media gateway or a media server to host IVR applications using 
simple scripts or XML to control the callflow. 

We support various communication technologies such as SIP, H.323, IAX2 and GoogleTalk making 
it easy to interface with other open source PBX systems such as sipX, OpenPBX, Bayonne, YATE or Asterisk.

We also support both wide and narrow band codecs making it an ideal solution to bridge legacy 
devices to the future. The voice channels and the conference bridge module all can operate 
at 8, 16 or 32 kilohertz and can bridge channels of different rates.

FreeSWITCH runs on several operating systems including Windows, Max OS X, Linux, BSD and Solaris 
on both 32 and 64 bit platforms.

Our developers are heavily involved in open source and have donated code and other resources to 
other telephony projects including sipXecs, OpenSER, Asterisk, CodeWeaver and OpenPBX.

%package devel
Summary:        Development package for FreeSWITCH open source telephony platform
Group:          System/Libraries
Requires:       %{name} = %{version}-%{release}

%description devel
FreeSWITCH development files

%package codec-passthru-amr
Summary:        Pass-through AMR Codec support for FreeSWITCH open source telephony platform
Group:          System/Libraries
Requires:       %{name} = %{version}-%{release}
Conflicts:	codec-amr

%description codec-passthru-amr
Pass-through AMR Codec support for FreeSWITCH open source telephony platform

%package codec-passthru-g723_1
Summary:        Pass-through g723.1 Codec support for FreeSWITCH open source telephony platform
Group:          System/Libraries
Requires:       %{name} = %{version}-%{release}
Conflicts:	codec-g723_1

%description codec-passthru-g723_1
Pass-through g723.1 Codec support for FreeSWITCH open source telephony platform

%package codec-passthru-g729
Summary:        Pass-through g729 Codec support for FreeSWITCH open source telephony platform
Group:          System/Libraries
Requires:       %{name} = %{version}-%{release}
Conflicts:	codec-g729

%description codec-passthru-g729
Pass-through g729 Codec support for FreeSWITCH open source telephony platform

%prep
%setup -q
%patch

%build
%ifos linux
%if 0%{?suse_version} > 1000 && 0%{?suse_version} < 1030
export CFLAGS="$CFLAGS -fstack-protector"
%endif
%if 0%{?fedora} >= 8
export QA_RPATHS=$[ 0x0001|0x0002 ]
%endif
%endif

PASSTHRU_CODEC_MODULES="codecs/mod_g729 codecs/mod_g723_1 codecs/mod_amr"
SPIDERMONKEY_MODULES=""
APPLICATIONS_MODULES="applications/mod_commands applications/mod_conference applications/mod_dptools applications/mod_enum applications/mod_esf applications/mod_expr applications/mod_fifo applications/mod_limit applications/mod_rss applications/mod_valet_parking applications/mod_voicemail applications/mod_fsv"
ASR_TTS_MODULES=""
CODECS_MODULES="codecs/mod_bv codecs/mod_celt codecs/mod_h26x codecs/mod_ilbc codecs/mod_siren codecs/mod_skel_codec codecs/mod_speex codecs/mod_voipcodecs"
DIALPLANS_MODULES="dialplans/mod_dialplan_asterisk dialplans/mod_dialplan_directory dialplans/mod_dialplan_xml"
DIRECTORIES_MODULES=
DOTNET_MODULES=
ENDPOINTS_MODULES="endpoints/mod_dingaling endpoints/mod_iax endpoints/mod_portaudio endpoints/mod_sofia ../../libs/openzap/mod_openzap endpoints/mod_loopback"
EVENT_HANDLERS_MODULES="event_handlers/mod_event_multicast event_handlers/mod_event_socket event_handlers/mod_cdr_csv"
FORMATS_MODULES="formats/mod_file_string formats/mod_local_stream formats/mod_native_file formats/mod_portaudio_stream formats/mod_shell_stream formats/mod_sndfile formats/mod_tone_stream"
LANGUAGES_MODULES=""
LOGGERS_MODULES="loggers/mod_console loggers/mod_logfile loggers/mod_syslog"
SAY_MODULES=""
TIMERS_MODULES=
DISABLED_MODULES="applications/mod_soundtouch directories/mod_ldap languages/mod_java languages/mod_python languages/mod_spidermonkey_skel ast_tts/mod_cepstral asr_tts/mod_lumenvox event_handlers/mod_event_test event_handlers/mod_radius_cdr event_handlers/mod_zeroconf formats/mod_shout say/mod_say_en say/mod_say_it say/mod_say_es say/mod_say_nl"
XML_INT_MODULES="xml_int/mod_xml_rpc  xml_int/mod_xml_curl xml_int/mod_xml_cdr"
MYMODULES="$PASSTHRU_CODEC_MODULES $SPIDERMONKEY_MODULES $APPLICATIONS_MODULES $ASR_TTS_MODULES $CODECS_MODULES $DIALPLANS_MODULES $DIRECTORIES_MODULES $DOTNET_MODULES $ENDPOINTS_MODULES $EVENT_HANDLERS_MODULES $FORMATS_MODULES $LANGUAGES_MODULES $LOGGERS_MODULES $SAY_MODULES $TIMERS_MODULES $XML_INT_MODULES"

export MODULES=$MYMODULES
test ! -f  modules.conf || rm -f modules.conf
touch modules.conf
for i in $MODULES; do echo $i >> modules.conf; done
export VERBOSE=yes
export DESTDIR=$RPM_BUILD_ROOT/
export PKG_CONFIG_PATH=/usr/bin/pkg-config:$PKG_CONFIG_PATH
export ACLOCAL_FLAGS="-I /usr/share/aclocal"

if test ! -f Makefile.in 
then 
   ./bootstrap.sh
fi


	%configure -C \
                --prefix=%{prefix} \
		--bindir=%{prefix}/bin \
		--libdir=%{prefix}/lib \
                --sysconfdir=%{prefix}/conf \
                --infodir=%{_infodir} \
                --mandir=%{_mandir} \
		--enable-core-libedit-support \
		--enable-core-odbc-support \
%ifos linux
%if 0%{?fedora} >= 8
                --without-libcurl \
%else
                --with-libcurl \
%endif
%endif
                --with-openssl \
		%{?configure_options}

#Create the version header file here
cat src/include/switch_version.h.in | sed "s/@SWITCH_VERSION_REVISION@/%{release}/g" > src/include/switch_version.h
touch .noversion

%{__make}

%install
# delete langugages
rm -rf conf/lang/en
rm -rf conf/lang/de
rm -rf conf/lang/fr
rm -rf conf/lang/ru
rm -rf $RPM_BUILD_ROOT%{prefix}/conf/lang/en
rm -rf $RPM_BUILD_ROOT%{prefix}/conf/lang/de
rm -rf $RPM_BUILD_ROOT%{prefix}/conf/lang/fr
rm -rf $RPM_BUILD_ROOT%{prefix}/conf/lang/ru

%{__make} DESTDIR=$RPM_BUILD_ROOT install

# Create a log dir
%{__mkdir} -p $RPM_BUILD_ROOT%{prefix}/log

%ifos linux
#Install the library path config so the system can find the modules
%{__mkdir} -p $RPM_BUILD_ROOT/etc/ld.so.conf.d
%{__cp} build/freeswitch.ld.so.conf $RPM_BUILD_ROOT/etc/ld.so.conf.d/

## # Install init files
## # On SuSE:
## %if 0%{?suse_version} > 100
## %{__install} -D -m 744 build/freeswitch.init.suse $RPM_BUILD_ROOT/etc/init.d/freeswitch
## %else
## # On RedHat like
## %{__install} -D -m 744 build/freeswitch.init.redhat $RPM_BUILD_ROOT/etc/init.d/freeswitch
## %endif
## # On SuSE make /usr/sbin/rcfreeswitch a link to /etc/init.d/freeswitch
## %if 0%{?suse_version} > 100
## %{__mkdir} -p $RPM_BUILD_ROOT/usr/sbin
## %{__ln_s} -f /etc/init.d/freeswitch $RPM_BUILD_ROOT/usr/sbin/rcfreeswitch
## %endif

# Add the sysconfiguration file
%{__install} -D -m 744 build/freeswitch.sysconfig $RPM_BUILD_ROOT/etc/sysconfig/freeswitch
# Add monit file
%{__install} -D -m 644 build/freeswitch.monitrc $RPM_BUILD_ROOT/etc/monit.d/freeswitch.monitrc
%endif


# Add a freeswitch user with group daemon
%pre
#%ifos linux
#/usr/sbin/useradd -r -g daemon -s /bin/false -c "The FreeSWITCH Open Source Voice Platform" -d %{prefix} freeswitch 2> /dev/null || :
#%endif

%post
%{?run_ldconfig:%run_ldconfig}
# Make FHS2.0 happy
#%{__mkdir} -p /etc/opt
#%{__ln_s} -f %{prefix}/conf /etc%{prefix}

##chkconfig --add freeswitch

%postun
%{?run_ldconfig:%run_ldconfig}
#%{__rm} -rf %{prefix}
#userdel freeswitch

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%files
%defattr(0644, %{fs_user},daemon, 0755)
%ifos linux
%dir /etc/monit.d
%endif
%dir %{prefix}/
%dir %{prefix}/bin
%dir %{prefix}/lib
%dir %{prefix}/lib/pkgconfig
%dir %{prefix}/mod
%dir %{prefix}/db
%dir %{prefix}/log
%dir %{prefix}/log/xml_cdr
%dir %{prefix}/htdocs
%dir %{prefix}/scripts
%dir %{prefix}/conf
%dir %{prefix}/conf/autoload_configs
%dir %{prefix}/conf/dialplan
%dir %{prefix}/conf/directory
%dir %{prefix}/conf/directory/default
%dir %{prefix}/conf/sip_profiles
%dir %{prefix}/conf/dialplan/default
%dir %{prefix}/conf/dialplan/public
%dir %{prefix}/conf/sip_profiles/internal
%dir %{prefix}/conf/sip_profiles/external
%dir %{prefix}/conf/jingle_profiles
%dir %{prefix}/conf/mrcp_profiles
%ifos linux
%config(noreplace) /etc/monit.d/freeswitch.monitrc
%endif
%config(noreplace) %{prefix}/conf/mime.types
%config(noreplace) %{prefix}/conf/*.tpl
%config(noreplace) %{prefix}/conf/*.ttml
%config(noreplace) %{prefix}/conf/*.xml
%config(noreplace) %{prefix}/conf/*.conf
%config(noreplace) %{prefix}/conf/autoload_configs/*
%config(noreplace) %{prefix}/conf/dialplan/*.xml
%config(noreplace) %{prefix}/conf/dialplan/default/*.xml
%config(noreplace) %{prefix}/conf/dialplan/public/*.xml
%config(noreplace) %{prefix}/conf/directory/*.xml
%config(noreplace) %{prefix}/conf/directory/default/*
%config(noreplace) %{prefix}/conf/sip_profiles/*.xml
%config(noreplace) %{prefix}/conf/sip_profiles/internal/*.xml
%config(noreplace) %{prefix}/conf/sip_profiles/external/*.xml
%config(noreplace) %{prefix}/conf/jingle_profiles/*.xml
%config(noreplace) %{prefix}/conf/mrcp_profiles/*.xml
%config(noreplace) %{prefix}/htdocs/*
%ifos linux
/etc/ld.so.conf.d/*
## /etc/init.d/freeswitch
/etc/sysconfig/freeswitch
## %if 0%{?suse_version} > 100
## /usr/sbin/rcfreeswitch
## %endif
%endif
%attr(0755,%{fs_user},daemon) %{prefix}/bin/*
%{prefix}/lib/libfreeswitch*.so*
%{prefix}/lib/libopenzap.so*
%{prefix}/lib/pkgconfig/*
%{prefix}/mod/mod_console.so*
%{prefix}/mod/mod_logfile.so*
%{prefix}/mod/mod_syslog.so*
%{prefix}/mod/mod_commands.so*
%{prefix}/mod/mod_conference.so*
%{prefix}/mod/mod_dptools.so*
%{prefix}/mod/mod_enum.so*
%{prefix}/mod/mod_esf.so*
%{prefix}/mod/mod_expr.so*
%{prefix}/mod/mod_fifo.so*
%{prefix}/mod/mod_file_string.so*
%{prefix}/mod/mod_limit.so*
%{prefix}/mod/mod_rss.so*
%{prefix}/mod/mod_valet_parking.so*
%{prefix}/mod/mod_voicemail.so*
%{prefix}/mod/mod_bv.so* 
%{prefix}/mod/mod_celt.so* 
%{prefix}/mod/mod_ilbc.so* 
%{prefix}/mod/mod_h26x.so*
%{prefix}/mod/mod_siren.so*
%{prefix}/mod/mod_voipcodecs.so* 
%{prefix}/mod/mod_skel_codec.so* 
%{prefix}/mod/mod_speex.so* 
%{prefix}/mod/mod_dialplan_directory.so* 
%{prefix}/mod/mod_dialplan_xml.so* 
%{prefix}/mod/mod_dialplan_asterisk.so* 
%{prefix}/mod/mod_dingaling.so* 
%{prefix}/mod/mod_iax.so* 
%{prefix}/mod/mod_portaudio.so* 
%{prefix}/mod/mod_portaudio_stream.so* 
%{prefix}/mod/mod_shell_stream.so* 
%{prefix}/mod/mod_sofia.so* 
%{prefix}/mod/mod_openzap.so* 
%{prefix}/mod/mod_loopback.so* 
%{prefix}/mod/mod_cdr_csv.so*
%{prefix}/mod/mod_event_multicast.so* 
%{prefix}/mod/mod_event_socket.so* 
%{prefix}/mod/mod_native_file.so* 
%{prefix}/mod/mod_sndfile.so* 
%{prefix}/mod/mod_local_stream.so* 
%{prefix}/mod/mod_xml_rpc.so* 
%{prefix}/mod/mod_xml_curl.so* 
%{prefix}/mod/mod_xml_cdr.so* 
%{prefix}/mod/mod_fsv.so
%{prefix}/mod/mod_tone_stream.so
%{prefix}/mod/ozmod_analog.so*
%{prefix}/mod/ozmod_analog_em.so*
%{prefix}/mod/ozmod_isdn.so*
%{prefix}/mod/ozmod_skel.so*
%{prefix}/mod/ozmod_zt.so*

%files codec-passthru-amr
%defattr(0644, %{fs_user},daemon, 0755)
%{prefix}/mod/mod_amr.so*

%files codec-passthru-g723_1
%defattr(0644, %{fs_user},daemon, 0755)
%{prefix}/mod/mod_g723_1.so*

%files codec-passthru-g729
%defattr(0644, %{fs_user},daemon, 0755)
%{prefix}/mod/mod_g729.so*

%files devel
%defattr(0644, %{fs_user},daemon, 0755)
%dir %{prefix}/lib
%dir %{prefix}/mod
%dir %{prefix}/include
%{prefix}/lib/*.a
%{prefix}/lib/*.la
%{prefix}/mod/*.a
%{prefix}/mod/*.la
%{prefix}/include/*.h
