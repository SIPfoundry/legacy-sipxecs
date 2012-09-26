include config/utility.am
include config/project.am

@PACKAGE@_EXTRA_DIST = \
	$(PACKAGE_META) \
	$(libexec_SCRIPTS:=.in)

PACKAGE_META = \
	$(cd $(srcdir); find -name 'packages-*')

PACKAGE_ID = \
	$(shell rpm --eval '%{?fedora:Fedora_%{fedora}}%{?rhel:CentOS_%{rhel}}-%{_arch}')

libexecdir= $(SIPX_LIBEXECDIR)
libexec_SCRIPTS = \
	sipx-package-info

confdir = $(SIPX_CONFDIR)
dist_conf_DATA = release

# For distributions not declared here
packages-$(PACKAGE_ID) :
	touch $@

$(libexec_SCRIPTS) : % : %.in Makefile
	@$(call SearchAndReplace,$<,$@)

install-data-hook: packages-$(PACKAGE_ID)
	$(INSTALL_DATA) -D packages-$(PACKAGE_ID) $(DESTDIR)$(SIPX_CONFDIR)/packages

CLEANFILES = \
	$(libexec_SCRIPTS)

