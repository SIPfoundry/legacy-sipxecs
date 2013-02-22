mongodb_VER = 2.2.3
mongodb_REL = 1
mongodb_SRPM = mongodb-$(mongodb_VER)-$(mongodb_REL)$(RPM_DIST).src.rpm
mongodb_SPEC = $(SRC)/$(PROJ)/mongodb.spec
mongodb_SOURCES = \
	$(SRC)/$(PROJ)/mongodb-src-r2.2.3.tar.gz \
	$(SRC)/$(PROJ)/mongodb-2.2.0-boost-filesystem3.patch \
	$(SRC)/$(PROJ)/mongodb-2.2.0-fix-xtime.patch \
	$(SRC)/$(PROJ)/mongodb-2.2.0-full-flag.patch \
	$(SRC)/$(PROJ)/mongodb-2.2.0-no-term.patch \
	$(SRC)/$(PROJ)/mongodb-2.2.0-shared-library.patch \
	$(SRC)/$(PROJ)/mongodb-2.2.0-use-system-version.patch \
	$(SRC)/$(PROJ)/mongodb.conf \
	$(SRC)/$(PROJ)/mongodb.init \
	$(SRC)/$(PROJ)/mongodb.logrotate \
	$(SRC)/$(PROJ)/mongodb.spec \
	$(SRC)/$(PROJ)/mongodb-tmpfile \
	$(SRC)/$(PROJ)/mongod.service \
	$(SRC)/$(PROJ)/mongod.sysconf

# targets not defined, nothing to do
mongodb.autoreconf mongodb.configure mongodb.dist:;
