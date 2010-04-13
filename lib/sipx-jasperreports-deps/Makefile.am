NAME=sipx-jasperreports-deps
VERSION = 1.0.0
RELEASE = 2

JAR_DIR_PREFIX=deps-jars

JASPER_DEPS_JARS = \
	$(JAR_DIR_PREFIX)/jasperreports-3.0.0.jar \
	$(JAR_DIR_PREFIX)/itext_1.5.4.jar \
	$(JAR_DIR_PREFIX)/jakarta-poi-3.0.2.jar \
	$(JAR_DIR_PREFIX)/jfreechart-1.0.10.jar \
	$(JAR_DIR_PREFIX)/jcommon-1.0.13.jar

TARBALL=$(NAME)-$(VERSION)-$(RELEASE).tar.gz

RPM = \
	$(DEST_RPM)/$(NAME)-$(VERSION)-$(RELEASE).noarch.rpm

RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

all : $(RPM)

$(RPM_SOURCES_DIR)/$(TARBALL) : $(JASPER_DEPS_JARS)
	$(AMTAR) -czf $(RPM_SOURCES_DIR)/$(TARBALL) -C $(srcdir) $(JASPER_DEPS_JARS)

$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: $(srcdir)/$(NAME).spec $(RPM_SOURCES_DIR)/$(TARBALL)
	rpmbuild -ba $(srcdir)/$(NAME).spec

$(RPM): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/noarch/`basename $@` $@
