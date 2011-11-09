include $(top_srcdir)/config/utility.am

SUBDIRS = ssl-cert

INCLUDES = -I$(top_srcdir)/include

utildir = @SIPX_LIBEXECDIR@
util_SCRIPTS = \
	sipx-utils.sh

bin_SCRIPTS = \
	sipx-validate-xml \
	sipxmongod \
	pgpatch.sh \
	sipx-dns

dist_bin_SCRIPTS = \
	pkg-upgrade \
	configmerge \
	configquery

setupdir = @SIPX_LIBEXECDIR@/setup.d
setup_SCRIPTS = \
        sipx-time-manager-init \
        sipx-change-timezone-init \
        sipx-mongodadmin \
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

noinst_PROGRAMS = \
	sipxconfigrpc \
	sipxconfigrpcsrv

sipxconfigrpc_SOURCES = \
	configrpc.cpp

sipxconfigrpcsrv_SOURCES = \
	configrpcsrv.cpp

sipxconfigrpc_LDADD = \
	../src/libsipXcommserver.la

sipxconfigrpcsrv_LDADD = \
	../src/libsipXcommserver.la
