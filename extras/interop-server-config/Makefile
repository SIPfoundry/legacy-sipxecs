# The name by which the update-passtokens script can be executed.
# This will probably have to be specified, as update-passtokens is not
# usually installed.
# It is available in the source as sipXcommserverLib/doc/update-passtokens.
UPDATE_PASSTOKENS=update-passtokens

# The names of the destination directories.
# These values are the defaults for RPM installation, but can be retargeted.
# See http://www.sipfoundry.org/sipX/doc/filesystem.html.
# Note that changing these values is not completely supported yet --
# some of the installed files would need to be adjusted.
SIPX_USER=sipx
SIPX_BINDIR=/usr/bin
SIPX_CONFDIR=/etc/sipxpbx
SIPX_DATADIR=/var/sipxdata
SIPX_LIBDIR=/usr/lib
SIPX_VARDIR=/var
SIPX_LOGDIR=/var/log/sipxpbx
SIPX_RUNDIR=/var/run/sipxpbx
HTTP_ROOTDIR=/usr/share/www/doc
HTTP_MODULEDIR=/etc/httpd/modules

# Determine the names of installed files based on the names of fles in the
# http_rootdir, sipx_confdir, and sipx_dbdir directories.

# The installed names of files without translation from .pre forms.
# This includes installed files whose distributed form is not .pre, as well
# as .pre files after being copied to the install directories.
CONFIG_FILES=$(subst sipx_confdir,$(SIPX_CONFDIR),$(filter-out %~,$(wildcard sipx_confdir/*)))
DOC_FILES=$(subst http_rootdir,$(HTTP_ROOTDIR),$(filter-out %~,$(wildcard http_rootdir/*)))
SIPDB_FILES=$(subst sipx_dbdir,$(SIPX_DATADIR)/sipdb,$(filter-out %~,$(wildcard sipx_dbdir/*)))

# The installed names of files that are translated from .pre forms.
CONFIG_FILES_EDITED=$(subst .pre,,$(subst sipx_confdir,$(SIPX_CONFDIR),$(wildcard sipx_confdir/*.pre)))
DOC_FILES_EDITED=$(subst .pre,,$(subst http_rootdir,$(HTTP_ROOTDIR),$(wildcard http_rootdir/*.pre)))
SIPDB_FILES_EDITED=$(subst .pre,,$(subst sipx_dbdir,$(SIPX_DATADIR)/sipdb,$(wildcard sipx_dbdir/*.pre)))

install: $(CONFIG_FILES) $(DOC_FILES) $(SIPDB_FILES) $(CONFIG_FILES_EDITED) $(DOC_FILES_EDITED) $(SIPDB_FILES_EDITED) $(SIPX_DATADIR)/sipdb/credential.xml

# Print the lists of files.
debug:
	@ echo CONFIG_FILES=$(CONFIG_FILES)
	@ echo DOC_FILES=$(DOC_FILES)
	@ echo SIPDB_FILES=$(SIPDB_FILES)
	@ echo DOC_FILES_EDITED=$(DOC_FILES_EDITED) 
	@ echo CONFIG_FILES_EDITED=$(CONFIG_FILES_EDITED)
	@ echo SIPDB_FILES_EDITED=$(SIPDB_FILES_EDITED)

$(CONFIG_FILES): $(SIPX_CONFDIR)/%: sipx_confdir/%
	cp $< $@
	chmod a+r $@

$(DOC_FILES): $(HTTP_ROOTDIR)/%: http_rootdir/%
	cp $< $@
	chmod a+r $@

$(SIPDB_FILES): $(SIPX_DATADIR)/sipdb/%: sipx_dbdir/%
	cp $< $@
	chmod a+r $@

$(SIPX_CONFDIR)/config.defs: $(SIPX_CONFDIR)/config.defs.in
	sh -c \
		"sed \
			-e 's!@SIPX_USER@!$(SIPX_USER)!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DATADIR@!$(SIPX_DATADIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@SIPX_VARDIR@!$(SIPX_VARDIR)!g' \
			-e 's!@SIPX_LOGDIR@!$(SIPX_LOGDIR)!g' \
			-e 's!@SIPX_RUNDIR@!$(SIPX_RUNDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HTTP_MODULEDIR@!$(HTTP_MODULEDIR)!g' \
		<$< >$@"
	chmod a+r $@

$(CONFIG_FILES_EDITED) $(DOC_FILES_EDITED) $(SIPDB_FILES_EDITED): %: %.pre $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	sh -c \
		"sed \
			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
			-e 's!@INTEROP_ALIAS_UDP@!$$INTEROP_ALIAS_UDP!g' \
			-e 's!@INTEROP_ALIAS_TCP@!$$INTEROP_ALIAS_TCP!g' \
			-e 's!@HOSTNAME@!$$MY_FULL_HOSTNAME!g' \
			-e 's!@SIPX_USER@!$(SIPX_USER)!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DATADIR@!$(SIPX_DATADIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@SIPX_VARDIR@!$(SIPX_VARDIR)!g' \
			-e 's!@SIPX_LOGDIR@!$(SIPX_LOGDIR)!g' \
			-e 's!@SIPX_RUNDIR@!$(SIPX_RUNDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HTTP_MODULEDIR@!$(HTTP_MODULEDIR)!g' \
		<$< >$@"
	chmod a+r $@
	[[ $@ != *.cgi ]] || chmod a+x $@

$(SIPX_DATADIR)/sipdb/credential.xml: $(SIPX_DATADIR)/sipdb/credential.xml.in $(SIPX_CONFDIR)/config.defs
	sh -c \
		"yes 1234 | $(UPDATE_PASSTOKENS) $< $@"
	@echo

.PHONY: install debug
