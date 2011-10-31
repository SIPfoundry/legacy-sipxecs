mongo_VER = 2.0.0
mongo_SRPM = mongo-$(mongo_VER)-mongodb_1$(RPM_DIST).src.rpm
mongo_SPEC = $(SRC)/$(PROJ)/mongo.spec
mongo_SOURCES = \
	mongodb-src-r$(mongo_VER).tar.gz

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist :;
