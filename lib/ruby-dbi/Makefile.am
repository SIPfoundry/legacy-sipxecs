VER = 0.1.1
REL = 2
ARCH=noarch

RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

TARBALL=dbi-$(VER).tar.gz
PATCH = dbi-0.1.0_install.patch

SRC = $(LIBSRC)/ruby-dbi/$(TARBALL)
DBI_URL = http://rubyforge.org/frs/download.php/12368

RPM = $(DEST_RPM)/ruby-dbi-$(VER)-$(REL).noarch.rpm

all : $(RPM)

$(SRC) :
	$(DOWNLOAD_FILE) $@ $(DBI_URL)/$(TARBALL)

$(RPM_SOURCES_DIR)/$(TARBALL) : $(SRC) 
	cp $< $@

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

$(RPM) : $(RPM_SOURCES_DIR)/$(PATCH) $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba $(srcdir)/ruby-dbi.spec
	cp @RPMBUILD_TOPDIR@/RPMS/noarch/$(@F) $@

