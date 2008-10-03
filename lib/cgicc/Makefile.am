CGICC_VER = 3.2.7
CGICC_REL = 2

SRC = $(srcdir)/src/cgicc-$(CGICC_VER).tar.gz
SRPM = $(DEST_SRPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).src.rpm
RPMS = \
	$(DEST_RPM)/cgicc-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm \
	$(DEST_RPM)/cgicc-devel-$(CGICC_VER)-$(CGICC_REL).@RPM_TARGET_ARCH@.rpm
RPM_SOURCES_DIR=@RPMBUILD_TOPDIR@/SOURCES

SPEC_FILE = $(srcdir)/src/cgicc.spec

all : PRINT_HEADER $(SRPM) $(RPMS) PRINT_FOOTER


$(RPM_SOURCES_DIR)/% : %
	cp $< $@

.PHONY: build-rpms
build-rpms: 
	cp $(SRC) @RPMBUILD_TOPDIR@/SOURCES
	cp $(srcdir)/src/*patch @RPMBUILD_TOPDIR@/SOURCES
	rpmbuild -ba --target @RPM_TARGET_ARCH@-none-linux $(SPEC_FILE)

$(SRPM): build-rpms
	mv @RPMBUILD_TOPDIR@/SRPMS/`basename $@` $@

$(RPMS): build-rpms
	mv @RPMBUILD_TOPDIR@/RPMS/@RPM_TARGET_ARCH@/`basename $@` $@

.PHONY: PRINT_HEADER
PRINT_HEADER :
	@printf "\n================================================================================\n"
	@printf "Building Package:\n"
	@printf "\tNAME = %s\n" "cgicc"
	@printf "\tVERSION = %s\n" "$(CGICC_VER)"
	@printf "\tRELEASE = %s\n" "$(CGICC_REL)"
	@printf "\tARCH = %s\n" "@RPM_TARGET_ARCH@"
	@printf "\tStarted building of %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "================================================================================\n"

.PHONY: PRINT_FOOTER
PRINT_FOOTER :
	@printf "\n================================================================================\n"
	@printf "Finished Building Package %s at %s\n" "$(PACKAGE)" "$(shell date)"
	@printf "\tBinary RPMS can be found under %s\n" "@RPMBUILD_TOPDIR@"
	@printf "================================================================================\n"
