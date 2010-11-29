boost_SRPM = boost-1.39.0-1.src.rpm
boost_SPEC = $(SRC)/$(PROJ)/boost.spec
boost_SOURCES = $(SRC)/$(PROJ)/boost_1_39_0.tar.bz2

# targets not defined, nothing to do
boost.autoreconf boost.configure boost.dist:;

