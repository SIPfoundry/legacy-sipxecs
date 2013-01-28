include $(top_srcdir)/config/utility.am

EXTRA_DIST = \
	$(tftp_DATA)

cfgdir = $(SIPX_CONFDIR)/polycom
dist_cfg_DATA = $(shell cd $(srcdir); find polycom -maxdepth 1 -type f )

cfgvmdir = $(SIPX_CONFDIR)/polycom/mac-address.d
dist_cfgvm_DATA = $(shell cd $(srcdir); find polycom/mac-address.d -type f)

cfgvm40dir = $(SIPX_CONFDIR)/polycom/mac-address.d.40
dist_cfgvm40_DATA = $(shell cd $(srcdir); find polycom/mac-address.d.40 -type f)

cfgvm32dir = $(SIPX_CONFDIR)/polycom/mac-address.d.32
dist_cfgvm32_DATA = $(shell cd $(srcdir); find polycom/mac-address.d.32 -type f)

cfgvm41dir = $(SIPX_CONFDIR)/polycom/mac-address.d.41
dist_cfgvm41_DATA = $(shell cd $(srcdir); find polycom/mac-address.d.41 -type f)

tftpdir = @SIPX_VARDIR@/configserver/phone/profile/tftproot
tftp_DATA = $(shell cd $(srcdir); find polycom/images -type f)
