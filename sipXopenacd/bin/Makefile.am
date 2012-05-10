include $(top_srcdir)/config/utility.am

snapshotdir = $(SIPX_BINDIR)/snapshot
dist_snapshot_SCRIPTS = \
	openacd-snapshot.sh

OPENACD_VARS = \
	$(filter OPENACD%,$(.VARIABLES))

$(dist_snapshot_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace, $<, $@, $(OPENACD_VARS))

DISTCLEANFILES = $(snapshot_SCRIPTS)
