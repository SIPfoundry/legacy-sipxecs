REL = 2_0_7
NAME = $(PACKAGE)

DOWNLOAD_DIR = $(LIBSRC)/$(NAME)
URL = http://dev.ctor.org/download
RPM = $(NAME)-$(VERSION)-$(REL).noarch.rpm
SRC = $(NAME)-$(REL).tar.gz

edit = sed \
	-e 's,@NAME\@,$(NAME),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@URL\@,$(URL),g' \
	-e 's,@REL\@,$(REL),g'

all : $(DEST_RPM)/$(RPM)

$(DOWNLOAD_DIR)/$(SRC) :
	$(DOWNLOAD_FILE) $@ $(URL)/$(SRC)

@RPMBUILD_TOPDIR@/SOURCES/$(SRC) : $(DOWNLOAD_DIR)/$(SRC)
	cp $< $@

$(NAME).spec : $(NAME).spec.in
	$(edit) $(srcdir)/$(NAME).spec.in > $(NAME).spec

@RPMBUILD_TOPDIR@/RPMS/noarch/$(RPM) : $(NAME).spec @RPMBUILD_TOPDIR@/SOURCES/$(SRC)
	rpmbuild -bb --target noarch $(NAME).spec

$(DEST_RPM)/$(RPM) : @RPMBUILD_TOPDIR@/RPMS/noarch/$(RPM)
	cp $< $@
