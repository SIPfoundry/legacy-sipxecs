lib += pymongo

pymongo_VER = 2.4.2
pymongo_SRPM = pymongo-$(pymongo_VER)-1$(RPM_DIST).src.rpm
pymongo_SPEC = $(SRC)/$(PROJ)/pymongo.spec
pymongo_SOURCES = pymongo-$(pymongo_VER).tar.gz

# targets not defined, nothing to do
pymongo.autoreconf pymongo.configure pymongo.dist:;
