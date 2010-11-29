nsis-data_SRPM = nsis-data-2.39-1.src.rpm
nsis-data_SPEC = $(SRC)/$(PROJ)/nsis-data.spec
nsis-data_SOURCES = $(SRC)/$(PROJ)/nsis-2.39.zip

# targets not defined, nothing to do
nsis-data.autoreconf nsis-data.configure nsis-data.dist:;

