# A prefix command applied to commands that manipulate the destination
# directories.  Left as null to prevent surprises for people who haven't
# read this Makefile before using it, but often set to "sudo".
SUDO=

# The names of the destination directories.
# These values are the defaults for RPM installation, but can be retargeted.
# See http://www.sipfoundry.org/sipX/doc/filesystem.html.
# Note that changing these values is not completely supported yet --
# some of the installed files would need to be adjusted.
SIPX_CONFDIR=/etc/sipxpbx
SIPX_DBDIR=/var/sipxdata/sipdb
HTTP_ROOTDIR=/usr/share/www/doc

# The names of all the installed files, derived from the files in
# the subdirectories.
CONFIG=$(subst sipx_confdir,$(SIPX_CONFDIR),$(filter-out %~,$(wildcard sipx_confdir/*)))
DOC=$(subst http_rootdir,$(HTTP_ROOTDIR),$(filter-out %~,$(wildcard http_rootdir/*)))
SIPDB=$(subst sipx_dbdir,$(SIPX_DBDIR),$(filter-out %~,$(wildcard sipx_dbdir/*)))

# index.html must be specified separately, because it is not a copy of
# a source file.
install: $(CONFIG) $(DOC) $(SIPDB) $(HTTP_ROOTDIR)/index.html

$(CONFIG): $(SIPX_CONFDIR)/%: sipx_confdir/%
	$(SUDO) cp $< $@

$(DOC): $(HTTP_ROOTDIR)/%: http_rootdir/%
	$(SUDO) cp $< $@

$(SIPDB): $(SIPX_DBDIR)/%: sipx_dbdir/%
	$(SUDO) cp $< $@

# The special rule to perform substitutions in index.html.in to create
# index.html.  This rule is done in the destination directory rather than
# in the source directory (1) so multiple installations can be done from
# one source directory, and (2) so if SIPXCHANGE_DOMAIN_NAME in config.defs
# needs to be changed after installation, re-running "make" will correct
# index.html.
$(HTTP_ROOTDIR)/index.html: $(HTTP_ROOTDIR)/index.html.in $(SIPX_CONFDIR)/config.defs
	. $(SIPX_CONFDIR)/config.defs ;\
	$(SUDO) sh -c \
		"sed -e 's/SIPXCHANGE_DOMAIN_NAME/$$SIPXCHANGE_DOMAIN_NAME/g' \
		<$< >$@"
