mongo_VER = 2.0.2
mongo_REV = 100
mongo_SRPM = mongodb-$(mongo_VER)-$(mongo_REV)$(RPM_DIST).src.rpm
mongo_SPEC = $(SRC)/$(PROJ)/mongodb.spec

# NOTE: there are 3 issues related to  mongo
#   http://jira.mongodb.org/browse/SERVER-4178
#   http://jira.mongodb.org/browse/SERVER-4179
#   http://jira.mongodb.org/browse/SERVER-4181
mongo_SOURCES = \
	mongodb-src-r$(mongo_VER).tar.gz

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist :;
