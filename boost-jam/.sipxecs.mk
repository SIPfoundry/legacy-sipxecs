boost-jam_SRPM = boost-jam-3.1.17-3.1.src.rpm
boost-jam_SPEC = $(SRC)/$(PROJ)/boost-jam.spec
boost-jam_SOURCES = $(SRC)/$(PROJ)/boost-jam-3.1.17.tgz $(SRC)/$(PROJ)/test.tgz

# targets not defined, nothing to do
boost-jam.autoreconf boost-jam.configure boost-jam.dist:;

