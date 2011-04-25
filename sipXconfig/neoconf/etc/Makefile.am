include $(top_srcdir)/config/subdir.am

all : $(conf_FILES)

EXTRA_DIST = \
	$(conf_SOURCE)

conf_SOURCE = \
	$(shell find $(srcdir) -type f -not \( \
	  -name 'Makefile*' \
	  \) -printf '%P\n') \
	VM_global_library.vm \
	sipxconfig.properties.in \
	sipxconfig.properties.map \
	setting.dtd \
	setting.xsl

conf_FILES = $(foreach F,$(conf_SOURCE),out/$F)
out/% : % Makefile
	test -d $(@D) || mkdir -p $(@D)
	$(info $< ->  $@)
	@$(call SearchAndReplace, $<,  $@)

install-data-hook : $(conf_FILES)
	echo "$$Installer" | \
	  InstallFileList="$(conf_SOURCE)" \
	  InstallSourceDir=out \
	  InstallDestDir=$(DESTDIR)$(SIPX_CONFDIR) \
	  bash
