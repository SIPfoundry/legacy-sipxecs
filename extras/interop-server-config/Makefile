# The name by which the setsipxpasswd script can be executed.
# This will probably have to be specified, as setsipxpasswd is not
# usually installed.
# It is available in the source as sipXcommserverLib/meta/setsipxpasswd,
# http://sipxecs.sipfoundry.org/rep/sipXecs/main/sipXcommserverLib/meta/setsipxpasswd
SETSIPXPASSWD=setsipxpasswd

# The user/group under which sipX runs.
SIPX_USER=sipx
SIPX_GROUP=sipx

# The names of the destination directories.
# These values are the defaults for RPM installation, but can be retargeted.
# See http://www.sipfoundry.org/sipX/doc/filesystem.html.
# Note that changing these values is not completely supported yet --
# some of the installed files would need to be adjusted.
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
CONFIG_FILES_EDITED=$(subst .pre,,$(subst sipx_confdir,$(SIPX_CONFDIR),$(wildcard sipx_confdir/*.pre))) $(SIPX_CONFDIR)/resource-lists.xml
DOC_FILES_EDITED=$(subst .pre,,$(subst http_rootdir,$(HTTP_ROOTDIR),$(wildcard http_rootdir/*.pre)))
SIPDB_FILES_EDITED=$(subst .pre,,$(subst sipx_dbdir,$(SIPX_DATADIR)/sipdb,$(wildcard sipx_dbdir/*.pre))) $(SIPX_DATADIR)/sipdb/credential.xml.in $(SIPX_DATADIR)/sipdb/permission.xml

# A shared secret value (144 random bits in base 64), in case domain-config needs to be generated.
SECRET:=$(shell openssl rand -base64 48)$(shell openssl rand -base64 48)$(shell openssl rand -base64 48)

install: $(CONFIG_FILES) $(DOC_FILES) $(SIPDB_FILES) \
	$(CONFIG_FILES_EDITED) $(DOC_FILES_EDITED) $(SIPDB_FILES_EDITED) \
	$(SIPX_DATADIR)/sipdb/credential.xml \
	$(HTTP_ROOTDIR)/sipx-version.cgi \
	$(SIPX_CONFDIR)/nattraversalrules.xml \
	$(SIPX_CONFDIR)/registrar-config \
	$(SIPX_CONFDIR)/sipXproxy-config

# Print the lists of files.
debug:
	@ echo CONFIG_FILES=$(CONFIG_FILES)
	@ echo DOC_FILES=$(DOC_FILES)
	@ echo SIPDB_FILES=$(SIPDB_FILES)
	@ echo DOC_FILES_EDITED=$(DOC_FILES_EDITED)
	@ echo CONFIG_FILES_EDITED=$(CONFIG_FILES_EDITED)
	@ echo SIPDB_FILES_EDITED=$(SIPDB_FILES_EDITED)

# These rules copy files to the destination directories.

$(CONFIG_FILES): $(SIPX_CONFDIR)/%: sipx_confdir/%
	cp $< $@
	chmod a+r $@

$(DOC_FILES): $(HTTP_ROOTDIR)/%: http_rootdir/%
	cp $< $@
	chmod a+r $@

$(SIPDB_FILES): $(SIPX_DATADIR)/sipdb/%: sipx_dbdir/%
	cp $< $@
	chmod a+r $@

# This rule performs substitutions on config.defs.in to generate config.defs.
$(SIPX_CONFDIR)/config.defs: $(SIPX_CONFDIR)/config.defs.in
	sh -c \
		"sed \
			-e 's!@SIPX_USER@!$(SIPX_USER)!g' \
			-e 's!@SIPX_GROUP@!$(SIPX_GROUP)!g' \
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

# This rule performs substitutions on .pre files to generate the non-.pre files.
$(CONFIG_FILES_EDITED) $(DOC_FILES_EDITED) $(SIPDB_FILES_EDITED): %: %.pre $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	sh -c \
		"sed \
			-e 's!@SIPXCHANGE_DOMAIN_NAME@!$$SIPXCHANGE_DOMAIN_NAME!g' \
			-e 's!@INTEROP_ALIAS_UDP@!$$INTEROP_ALIAS_UDP!g' \
			-e 's!@INTEROP_ALIAS_TCP@!$$INTEROP_ALIAS_TCP!g' \
			-e 's!@HOSTNAME@!$$MY_FULL_HOSTNAME!g' \
			-e 's!@SIPX_USER@!$(SIPX_USER)!g' \
			-e 's!@SIPX_GROUP@!$(SIPX_GROUP)!g' \
			-e 's!@SIPX_BINDIR@!$(SIPX_BINDIR)!g' \
			-e 's!@SIPX_CONFDIR@!$(SIPX_CONFDIR)!g' \
			-e 's!@SIPX_DATADIR@!$(SIPX_DATADIR)!g' \
			-e 's!@SIPX_LIBDIR@!$(SIPX_LIBDIR)!g' \
			-e 's!@SIPX_VARDIR@!$(SIPX_VARDIR)!g' \
			-e 's!@SIPX_LOGDIR@!$(SIPX_LOGDIR)!g' \
			-e 's!@SIPX_RUNDIR@!$(SIPX_RUNDIR)!g' \
			-e 's!@HTTP_ROOTDIR@!$(HTTP_ROOTDIR)!g' \
			-e 's!@HTTP_MODULEDIR@!$(HTTP_MODULEDIR)!g' \
			-e 's!@SECRET@!$(SECRET)!g' \
		<$< >$@"
	chmod a+r $@
	[[ $@ != *.cgi ]] || chmod a+x $@

# This rule sets the passwords and pintokens in credential.xml.in to generate
# credential.xml.
$(SIPX_DATADIR)/sipdb/credential.xml: $(SIPX_DATADIR)/sipdb/credential.xml.in $(SIPX_CONFDIR)/config.defs
	cp $< $@
	$(SETSIPXPASSWD) --credential $@ 1234
	chmod a+r $@

# This rule runs the script credential.xml.in.pre.script to generate
# credential.xml.in.pre.
$(SIPX_DATADIR)/sipdb/credential.xml.in.pre: $(SIPX_DATADIR)/sipdb/credential.xml.in.pre.script
	sh $< >$@

# This rule runs the script permission.xml.pre.script to generate
# permission.xml.pre.
$(SIPX_DATADIR)/sipdb/permission.xml.pre: $(SIPX_DATADIR)/sipdb/permission.xml.pre.script
	sh $< >$@

# This rule runs the script alias.xml.pre.script to generate
# alias.xml.pre.
$(SIPX_DATADIR)/sipdb/alias.xml.pre: $(SIPX_DATADIR)/sipdb/alias.xml.pre.script
	sh $< >$@

# This rule runs the script resource-lists.xml.pre.script to generate
# resource-lists.xml.pre.
$(SIPX_CONFDIR)/resource-lists.xml.pre: $(SIPX_CONFDIR)/resource-lists.xml.pre.script
	sh $< >$@

# Do substitutions from config.defs for files that no longer have them done by the .sh scripts.
$(SIPX_CONFDIR)/sipXproxy-config: $(SIPX_CONFDIR)/sipXproxy-config.in
	configpp --defs $(SIPX_CONFDIR)/config.defs --in $< --out $@
$(SIPX_CONFDIR)/registrar-config: $(SIPX_CONFDIR)/registrar-config.in
	configpp --defs $(SIPX_CONFDIR)/config.defs --in $< --out $@
$(SIPX_CONFDIR)/nattraversalrules.xml: $(SIPX_CONFDIR)/nattraversalrules.xml.in
	configpp --defs $(SIPX_CONFDIR)/config.defs --in $< --out $@

# This rule edits sipx-version.cgi.in to generate sipx-version.cgi.
$(HTTP_ROOTDIR)/sipx-version.cgi: $(HTTP_ROOTDIR)/sipx-version.cgi.in
	WCROOT="`svn info | grep '^URL:'`" ;\
	SVNVERSION=`svnversion` ;\
	sed \
		-e 's!@WCROOT@!'"$$WCROOT"'!g' \
		-e 's!@SVNVERSION@!'$$SVNVERSION'!g' \
		<$< >$@
	chmod a+r $@
# An additional dependency for sipx-version.cgi to force it to be regenerated
# every time config.defs is touched (which includes every time the
# configuration is updated by following the standard instructions).
$(HTTP_ROOTDIR)/sipx-version.cgi: $(SIPX_CONFDIR)/config.defs

.PHONY: install debug
