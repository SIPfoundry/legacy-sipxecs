include $(top_srcdir)/config/subdir.am

SUBDIRS = ssl-cert

INCLUDES = -I$(top_srcdir)/include

LOCAL_SHELLSCRIPTS = \
   sipx-validate-xml \
   pgpatch.sh \
   sipx-dns

bin_SCRIPTS = \
  $(LOCAL_SHELLSCRIPTS)

dist_bin_SCRIPTS = \
   ${PACKAGE}-config \
   pkg-upgrade \
   configmerge \
   configquery

setupbindir = @SIPX_LIBEXECDIR@/setup.d

setupbin_SCRIPTS = \
        sipx-time-manager-init \
        sipx-change-timezone-init \
        sipxpackage-init \
        sipxswadmin-init \
        sipxzoneadmin-init

setupbin_IN = $(foreach file,$(setupbin_SCRIPTS),$(file).in)

EXTRA_DIST = $(foreach sipxScript,$(LOCAL_SHELLSCRIPTS) $(setupbin_SCRIPTS),$(sipxScript).in)

noinst_PROGRAMS = \
	sipxconfigrpc \
	sipxconfigrpcsrv

$(foreach sipxScript,$(LOCAL_SHELLSCRIPTS),$(sipxScript)) : % : $(srcdir)/%.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
	chmod +x $@

sipxconfigrpc_SOURCES = \
	configrpc.cpp

sipxconfigrpcsrv_SOURCES = \
  configrpcsrv.cpp

sipxconfigrpc_LDADD = \
	../src/libsipXcommserver.la

sipxconfigrpcsrv_LDADD = \
	../src/libsipXcommserver.la

sipregistrar_CXXFLAGS = \
	-I$(top_builddir)/config \
	-I$(top_srcdir)/include \
	-DSIPX_LOGDIR=\"@SIPX_LOGDIR@\" \
	-DSIPX_RUNDIR=\"@SIPX_RUNDIR@\" \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\"

$(top_builddir)/BUILDSTAMP:
	${MAKE} -C $(top_builddir) BUILDSTAMP

${PACKAGE}-config: $(top_srcdir)/config/sipX-config.in $(top_builddir)/BUILDSTAMP
	@BuildStamp="$(shell cat $(top_builddir)/BUILDSTAMP)" \
	; ${LocalizeSipXconfig} -e "s/@SIPX_BUILDSTAMP\@/$${BuildStamp}/" \
	    $(top_srcdir)/config/sipX-config.in \
	  > ${PACKAGE}-config
	chmod +x ${PACKAGE}-config

$(setupbin_SCRIPTS) : $(setupbin_IN)
	@echo "Localize $< -> $@"
	@${LocalizeSipXconfig} $(srcdir)/$(notdir $@).in > $@

DISTCLEANFILES = sipx-validate-xml
