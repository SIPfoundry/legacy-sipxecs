include $(top_srcdir)/config/subdir.am

# I had trouble passing a enviroment variable to rake in a consistent
# way, the still requires builddir env to be set.
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

install-exec-hook : $(INSTALL_GEM)

uninstall-hook : uninstall-gem

.PHONY: package-gem
# see rakefile for details regarding multiple targets
package-gem :
	builddir=`pwd`; cd $(srcdir) && $(rake) client_bindings
	builddir=`pwd`; cd $(srcdir) && $(rake) package

uninstall-gem :
	GEM_PATH=$(GEM_LIB_DIR) \
	$(GEM) uninstall \
	  sipXconfig-report

# install gems now, this is the final host machine
.PHONY: install-gem
install-gem : package-gem
	$(mkdir_p) $(GEM_LIB_DIR)
	$(GEM) install \
	  --install-dir $(GEM_LIB_DIR) \
	  --local pkg/sipXconfig-report-*.gem

# gems are copied into installation and will be installed on host machine
# later controlled by %post section for rpm spec file
.PHONY: install-gem-for-rpm
install-gem-for-rpm : package-gem
	$(mkdir_p) $(DESTDIR)@SIPX_VARLIB@
	$(INSTALL) -m 644 pkg/sipXconfig-report-*.gem $(DESTDIR)@SIPX_VARLIB@

check :
	builddir=`pwd`; DB_USER=@POSTGRESQL_USER@; \
	cd $(srcdir) && $(rake)

$(DESTDIR)$(bindir)/sipxconfig-report : bin/sipxconfig-report.in
	mkdir -p bin
	@echo "Localize $< -> $@"
	@${LocalizeSipXconfig} -e 's,@GEM_LIB_DIR\@,$(GEM_LIB_DIR),g' \
	    $(srcdir)/bin/sipxconfig-report.in \
	      > bin/sipxconfig-report
	$(INSTALL) -D -m 755 bin/sipxconfig-report \
	    $(DESTDIR)$(bindir)/sipxconfig-report
