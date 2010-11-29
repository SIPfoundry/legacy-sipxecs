ruby-dbi_SRPM = ruby-dbi-0.1.1-2.src.rpm
ruby-dbi_SPEC = $(SRC)/$(PROJ)/ruby-dbi.spec
ruby-dbi_SOURCES = $(SRC)/$(PROJ)/dbi-0.1.1.tar.gz $(SRC)/$(PROJ)/dbi-0.1.0_install.patch

# targets not defined, nothing to do
ruby-dbi.autoreconf ruby-dbi.configure ruby-dbi.dist:;

