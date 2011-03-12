mongo_SRPM = mongodb-1.6.4-1.el5.src.rpm
#mongo_SPEC = $(SRC)/$(PROJ)/mongo.spec
#mongo_SOURCES = $(SRC)/$(PROJ)/mongodb-src-r1.6.3.tar.gz

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist :;

mongo.srpm :
	$(call CopySourceFile,$(mongo_SRPM),$(MOCK_SRPM_DIR))

