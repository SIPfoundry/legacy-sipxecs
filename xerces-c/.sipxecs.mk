xerces-c_SRPM = xerces-c-2.8.0-2.src.rpm
xerces-c_SPEC = $(SRC)/$(PROJ)/xerces-c.spec
xerces-c_SOURCES = $(SRC)/$(PROJ)/xerces-c-src_2_8_0.tar.gz $(SRC)/$(PROJ)/xerces-c.patch

# targets not defined, nothing to do
xerces-c.autoreconf xerces-c.configure xerces-c.dist:;

