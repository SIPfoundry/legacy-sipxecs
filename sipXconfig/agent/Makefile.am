SIPX_PROCESS_DEFS = \
	sipxconfig-agent

include $(top_srcdir)/config/subdir.am

rake=$(RAKE) builddir="$$builddir"
GEM_LIB_DIR=$(libdir)/ruby/gems/1.8

clean :
	builddir=`pwd`; \
	cd $(srcdir) && $(rake) clobber

if IS_RPMBUILD
INSTALL_GEM = install-gem-for-rpm
else
INSTALL_GEM = install-gem
endif

install : \
	$(DESTDIR)$(SIPX_DATADIR)/process.d/sipxconfig-agent-process.xml \
	$(INSTALL_GEM) \
	$(DESTDIR)$(bindir)/sipxconfig-agent

uninstall-hook : uninstall-gem

.PHONY: package-gem
package-gem :
	builddir=`pwd`; \
	cd $(srcdir) && $(rake) package && cd $(builddir)

uninstall-gem :
	GEM_PATH=$(GEM_LIB_DIR) \
	$(GEM) uninstall \
	  sipXconfig-agent

# install gems now, this is the final host machine
.PHONY: install-gem
install-gem : package-gem
	$(mkdir_p) $(GEM_LIB_DIR)
	$(GEM) install \
	  --install-dir $(GEM_LIB_DIR) \
	  --local pkg/sipXconfig-agent-*.gem

# gems are copied into installation and will be installed on host machine
# later
.PHONY: install-gem-for-rpm
install-gem-for-rpm : package-gem
	$(mkdir_p) $(DESTDIR)@SIPX_VARLIB@
	$(INSTALL) -m 644 pkg/sipXconfig-agent-*.gem $(DESTDIR)@SIPX_VARLIB@
	$(INSTALL) -m 644 $(srcdir)/meta/file-tail-*.gem $(DESTDIR)@SIPX_VARLIB@

check :
	cd $(srcdir) && $(rake)

$(DESTDIR)$(bindir)/sipxconfig-agent : sipxconfig-agent
	$(INSTALL) -D -m 755 sipxconfig-agent \
	    $(DESTDIR)$(bindir)/sipxconfig-agent

sipxconfig-agent : bin/sipxconfig-agent.in
	@echo "Localize $< -> $@"
	@${LocalizeSipXconfig} -e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g' \
	    $(srcdir)/bin/sipxconfig-agent.in \
	      > sipxconfig-agent
