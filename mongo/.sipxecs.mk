mongo_SRPM = mongodb-1.6.4-3$(RPM_DIST).src.rpm
mongo_SPEC = $(SRC)/$(PROJ)/mongodb.spec
mongo_SOURCES = \
	mongodb-src-r1.6.4.tar.gz \
	$(SRC)/$(PROJ)/mongodb.conf \
	$(SRC)/$(PROJ)/mongodb-cppflags.patch \
	$(SRC)/$(PROJ)/mongodb-client-ldflags.patch \
	$(SRC)/$(PROJ)/mongodb.init \
	$(SRC)/$(PROJ)/mongodb.logrotate

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist :;

#mongo.srpm :
#	$(call CopySourceFile,$(mongo_SRPM),$(MOCK_SRPM_DIR))

