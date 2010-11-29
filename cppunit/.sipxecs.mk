cppunit_SRPM = cppunit-1.12.1-2.src.rpm
cppunit_SPEC = $(SRC)/$(PROJ)/cppunit.spec
cppunit_SOURCES = $(SRC)/$(PROJ)/cppunit-1.12.1.tar.gz

# targets not defined, nothing to do
cppunit.autoreconf cppunit.configure cppunit.dist:;

