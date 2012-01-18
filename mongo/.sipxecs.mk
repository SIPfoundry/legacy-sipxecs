mongo_VER = 2.0.2
mongo_SRPM = mongodb-$(mongo_VER)-1$(RPM_DIST).src.rpm
mongo_SPEC = $(SRC)/$(PROJ)/mongodb.spec

# NOTE: tarfile is *not* pristine. there are 3 patches applied to it before creating tarball.
#   http://jira.mongodb.org/browse/SERVER-4178
#   http://jira.mongodb.org/browse/SERVER-4179
#   http://jira.mongodb.org/browse/SERVER-4181
# but assumption is these will be applied upstream and we won't have to maintain these patches.
mongo_SOURCES = \
	mongodb-src-r$(mongo_VER).tar.gz

# targets not defined, nothing to do
mongo.autoreconf mongo.configure mongo.dist :;
