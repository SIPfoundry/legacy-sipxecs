include $(top_srcdir)/config/utility.am

INCLUDES = -I$(top_srcdir)/include

utildir = @SIPX_LIBEXECDIR@
util_SCRIPTS = \
	sipx-archive-common \
	sipx-utils.sh

bin_SCRIPTS = \
	sipx-validate-xml \
	sipx-dns \
	sipx-backup \
	sipx-restore \
	sipx-snapshot

dist_bin_SCRIPTS = \
	pkg-upgrade \
	configmerge \
	configquery

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
        sipx-time-manager-init \
        sipx-change-timezone-init \
        sipxpackage-init \
        sipxswadmin-init \
        sipxcdrpostgres-init \
        sipxzoneadmin-init

EXTRA_DIST = \
	$(bin_SCRIPTS:=.in) \
	$(setup_SCRIPTS:=.in) \
	$(util_SCRIPTS:=.in)

$(util_SCRIPTS) $(bin_SCRIPTS) $(setup_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

CLEANFILES = $(bin_SCRIPTS) $(setup_SCRIPTS) $(util_SCRIPTS)
