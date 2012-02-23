include $(top_srcdir)/config/utility.am

FS_VARS = \
	$(filter FREESWITCH%,$(.VARIABLES))

cfdefaultsdir = $(SIPX_CFDATA)/defaults
cfdefaults_DATA = \
	sipxfreeswitch.cfdat

sipxfreeswitch.cfdat : Makefile
	! test -f $@ || rm $@
	$(foreach V,$(FS_VARS), echo '=$(V)=$($(V))' >> $@;)

cfinputsdir = $(SIPX_CFINPUTS)/plugin.d
dist_cfinputs_DATA = \
	sipxfreeswitch.cf

DISTCLEANFILES = $(cfdefaults_DATA)

fscfgdir = @SIPX_CONFDIR@/freeswitch/conf

autoloadcfgdir = @SIPX_CONFDIR@/freeswitch/conf/autoload_configs
dist_autoloadcfg_DATA = \
	autoload_configs/acl.conf.xml \
	autoload_configs/alsa.conf.xml \
	autoload_configs/callcenter.conf.xml \
	autoload_configs/cdr_csv.conf.xml \
	autoload_configs/cdr_pg_csv.conf.xml \
	autoload_configs/cidlookup.conf.xml \
	autoload_configs/conference.conf.xml \
	autoload_configs/console.conf.xml \
	autoload_configs/db.conf.xml \
	autoload_configs/dialplan_directory.conf.xml \
	autoload_configs/dingaling.conf.xml \
	autoload_configs/directory.conf.xml \
	autoload_configs/distributor.conf.xml \
	autoload_configs/easyroute.conf.xml \
	autoload_configs/enum.conf.xml \
	autoload_configs/erlang_event.conf.xml \
	autoload_configs/event_multicast.conf.xml \
	autoload_configs/event_socket.conf.xml \
	autoload_configs/fax.conf.xml \
	autoload_configs/fifo.conf.xml \
	autoload_configs/hash.conf.xml \
	autoload_configs/ivr.conf.xml \
	autoload_configs/java.conf.xml \
	autoload_configs/lcr.conf.xml \
	autoload_configs/local_stream.conf.xml \
	autoload_configs/logfile.conf.xml \
	autoload_configs/memcache.conf.xml \
	autoload_configs/modules.conf.xml \
	autoload_configs/nibblebill.conf.xml \
	autoload_configs/opal.conf.xml \
	autoload_configs/osp.conf.xml \
	autoload_configs/pocketsphinx.conf.xml \
	autoload_configs/portaudio.conf.xml \
	autoload_configs/post_load_modules.conf.xml \
	autoload_configs/redis.conf.xml \
	autoload_configs/rss.conf.xml \
	autoload_configs/sangoma_codec.conf.xml \
	autoload_configs/shout.conf.xml \
	autoload_configs/skinny.conf.xml \
	autoload_configs/sofia.conf.xml \
	autoload_configs/spandsp.conf.xml \
	autoload_configs/switch.conf.xml \
	autoload_configs/syslog.conf.xml \
	autoload_configs/timezones.conf.xml \
	autoload_configs/tts_commandline.conf.xml \
	autoload_configs/unicall.conf.xml \
	autoload_configs/unimrcp.conf.xml \
	autoload_configs/voicemail.conf.xml \
	autoload_configs/xml_cdr.conf.xml \
	autoload_configs/xml_curl.conf.xml \
	autoload_configs/xml_rpc.conf.xml \
	autoload_configs/zeroconf.conf.xml

