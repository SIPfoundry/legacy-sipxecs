RUBY_LIBSRC = $(LIBSRC)/ruby

RUBY_VER = 1.8.4
RUBY_ABI_VER = 1.8
RUBY_REL = 1

SRPM = $(RUBY_LIBSRC)/ruby-$(RUBY_VER)-$(RUBY_REL).c4.src.rpm
RPMS = \
	$(DEST_RPM)/ruby-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/ruby-libs-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/ruby-devel-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/irb-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/rdoc-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/ri-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/ruby-mode-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/ruby-tcltk-$(RUBY_VER)-$(RUBY_REL).c4.@RPM_TARGET_ARCH@.rpm

ABI_RPM = \
	$(DEST_RPM)/ruby-abi-$(RUBY_ABI_VER)-1.noarch.rpm

all : $(RPMS) $(ABI_RPM)

$(SRPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_RPM_URL)/SRPMS/`basename $@`

.PHONY:build-rpms
build-rpms : $(SRPM)
	rpmbuild --rebuild $(SRPM)

# Fullfills RUBY ABI requirement
# http://mail.madstop.com/pipermail/puppet-dev/2006-August/001503.html
$(ABI_RPM) : ruby-abi.spec
	rpmbuild -ba $<
	cp @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@

$(RPMS) : build-rpms
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
