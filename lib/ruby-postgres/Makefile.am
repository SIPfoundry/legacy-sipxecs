RUBY_AUX_RPMS_URL = http://people.redhat.com/dlutter/yum

PGSQL_LIBSRC = $(LIBSRC)/ruby-postgres

VER = 0.7.1
REL = 5
DISTRO = fc6

RPM = $(DEST_RPM)/ruby-postgres-$(VER)-$(REL).@RPM_TARGET_ARCH@.rpm
SRPM = $(PGSQL_LIBSRC)/ruby-postgres-$(VER)-$(REL).$(DISTRO).src.rpm

# Where rpmbuild will do its work.
all : $(RPM)

$(SRPM) :
	$(DOWNLOAD_FILE) $@ $(RUBY_AUX_RPMS_URL)/SRPMS/`basename $@`

$(RPM) : $(SRPM)
	rpmbuild --rebuild $(SRPM)
	cp @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@
