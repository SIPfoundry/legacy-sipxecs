VER = 0.7.1
REL = 2
ARCH=noarch

RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

TARBALL=ruby-postgres-$(VER).tar.gz

SRC = $(LIBSRC)/ruby-postgres/$(TARBALL)
RUBY_POSTGRES_URL = http://ruby.scripting.ca/postgres/archive

RPM = $(DEST_RPM)/ruby-postgres-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm

all : $(RPM)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(RUBY_POSTGRES_URL)/$(TARBALL)

$(RPM_SOURCES_DIR)/$(TARBALL) : $(SRC) 
	cp $< $@

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

$(RPM) : $(RPM_SOURCES_DIR)/$(PATCH) $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba $(srcdir)/ruby-postgres.spec
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/$(@F) $@

