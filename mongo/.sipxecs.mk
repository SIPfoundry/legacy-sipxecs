mongo_SRPM = mongo-1.6.3-mongodb_1$(RPM_DIST).src.rpm
mongo_SPEC = $(SRC)/$(PROJ)/mongo.spec
mongo_SOURCES = $(SRC)/$(PROJ)/mongodb-src-r1.6.3.tar.gz

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist:;

