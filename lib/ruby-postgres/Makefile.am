VER = 0.7.1
REL = 2
ARCH=noarch

RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

TARBALL=ruby-postgres-$(VER).tar.gz

RUBY_POSTGRES_URL = http://www.postgresql.jp/interfaces/ruby/archive/

RPM = $(DEST_RPM)/ruby-postgres-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm

all : $(RPM)

$(RPM_SOURCES_DIR)/$(TARBALL) :
	cp $(srcdir)/ruby-postgres-0.7.1.tar.gz $(RPM_SOURCES_DIR)/$(TARBALL)

$(RPM) : $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba ruby-postgres.spec
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/$(@F) $@

