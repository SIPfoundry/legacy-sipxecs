include $(top_srcdir)/config/utility.am

INCLUDES = -I$(top_srcdir)/include
libexecdir = @SIPX_LIBEXECDIR@
libexec_SCRIPTS = \
	sipx-strip-db-secrets.pl

utildir = @SIPX_LIBEXECDIR@
util_SCRIPTS = \
	sipx-archive-common \
	sipx-utils.sh

bin_SCRIPTS = \
	sipx-validate-xml \
	sipx-backup \
	sipx-restore \
	sipx-snapshot \
	sipx-ssl-symlinks.sh

dist_bin_SCRIPTS = \
	pkg-upgrade \
	configmerge \
	configquery

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
        sipxpackage-init \
        sipxswadmin-init \
        sipxcdrpostgres-init

EXTRA_DIST = \
	$(libexec_SCRIPTS:=.in) \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in) \
	$(util_SCRIPTS:=.in)

$(util_SCRIPTS) $(libexec_SCRIPTS) $(bin_SCRIPTS) $(setup_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS) $(util_SCRIPTS) $(libexec_SCRIPTS)
