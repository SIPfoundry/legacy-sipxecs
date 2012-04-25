include config/utility.am
include config/project.am

EXTRA_DIST = \
	tunnel.in

initddir = @SIPX_SERVICEDIR@
initd_SCRIPTS = \
	tunnel-server \
	tunnel-client

confdir = @SIPX_CONFDIR@/tunnel
dist_conf_DATA = \
	tunnel-server.erb \
	tunnel-client.erb

cfinputdir = @SIPX_CFINPUTS@/plugin.d
dist_cfinput_DATA = \
	tunnel.cf

tunnel-% : MODE = $(word 2,$(subst -, ,$@))
$(initd_SCRIPTS): tunnel.in
	@$(call SearchAndReplace,$<,$@,MODE)

CLEANFILES = $(inid_SCRIPTS)