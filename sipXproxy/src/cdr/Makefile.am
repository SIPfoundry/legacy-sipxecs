include $(top_srcdir)/config/subdir.am


rake=$(RAKE) builddir="$$builddir"
GEM_LIB_DIR=$(libdir)/ruby/gems/1.8

clean :
	builddir=$(srcdir); \
	cd $(srcdir) && $(rake) clobber

if IS_RPMBUILD
INSTALL_GEM = install-gem-for-rpm
else
INSTALL_GEM = install-gem
endif

install : $(INSTALL_GEM)

.PHONY: package-gem
package-gem :
	builddir=`pwd`; \
	cd $(srcdir) && $(rake) callresolver_version=2.0.0 package

.PHONY: install-gem
install-gem : package-gem
	$(mkdir_p) $(GEM_LIB_DIR)
	$(GEM) install \
	  --install-dir $(GEM_LIB_DIR) \
	  --local pkg/sipxcallresolver

# gems are copied into installation and will be installed on host machine
# later
.PHONY: install-gem-for-rpm
install-gem-for-rpm : package-gem
	echo "Installing call resolver gem"
	$(INSTALL) -d -m755 $(DESTDIR)@SIPX_VARLIB@
	$(INSTALL) -m 644 pkg/sipxcallresolver*.gem $(DESTDIR)@SIPX_VARLIB@

check :
	builddir=`pwd`; \
	cd $(srcdir) && $(rake)
