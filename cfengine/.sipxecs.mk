cfengine_REV = 1
cfengine_SRPM = cfengine-3.3.0-$(cfengine_REV)$(RPM_DIST).src.rpm
cfengine_SPEC = $(SRC)/$(PROJ)/cfengine.spec
cfengine_SOURCES = \
	cfengine-3.3.0.tar.gz \
	$(SRC)/$(PROJ)/cf-monitord \
	$(SRC)/$(PROJ)/cf-serverd \
	$(SRC)/$(PROJ)/cf-execd

# targets not defined, nothing to do
cfengine.autoreconf cfengine.configure cfengine.dist :;
