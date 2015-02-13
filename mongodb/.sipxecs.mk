mongodb_VER = 2.6.7
mongodb_REL = 1
mongodb_SRPM = mongodb-$(mongodb_VER)-$(mongodb_REL)$(RPM_DIST).src.rpm
mongodb_SPEC = $(SRC)/$(PROJ)/mongodb.spec
mongodb_SOURCES = \
	$(SRC)/$(PROJ)/mongodb-src-r$(mongodb_VER).tar.gz \
	$(SRC)/$(PROJ)/mongodb.conf \
	$(SRC)/$(PROJ)/mongod.init \
	$(SRC)/$(PROJ)/mongodb.logrotate \
	$(SRC)/$(PROJ)/mongodb.spec \
	$(SRC)/$(PROJ)/mongodb-tmpfile \
	$(SRC)/$(PROJ)/mongod.service \
	$(SRC)/$(PROJ)/mongod.sysconfig

mongodb.dist:;
