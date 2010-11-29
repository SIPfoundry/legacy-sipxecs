stunnel_SRPM = stunnel-4.33-1.src.rpm
stunnel_SPEC = $(SRC)/$(PROJ)/stunnel.spec
stunnel_SOURCES = $(SRC)/$(PROJ)/stunnel-4.33.tar.gz

# targets not defined, nothing to do
stunnel.autoreconf stunnel.configure stunnel.dist:;

