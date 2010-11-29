rrdtool_SRPM = rrdtool-1.2.26-5.src.rpm
rrdtool_SPEC = $(SRC)/$(PROJ)/rrdtool.spec
rrdtool_SOURCES = $(SRC)/$(PROJ)/rrdtool-1.2.26.tar.gz

# targets not defined, nothing to do
rrdtool.autoreconf rrdtool.configure rrdtool.dist:;

