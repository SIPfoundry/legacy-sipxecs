nsis_SRPM = nsis-2.39-1.src.rpm
nsis_SPEC = $(SRC)/$(PROJ)/nsis.spec
nsis_SOURCES = $(SRC)/$(PROJ)/nsis-2.39-src.tar.bz2 $(SRC)/$(PROJ)/nsis.patch

# targets not defined, nothing to do
nsis.autoreconf nsis.configure nsis.dist:;

