resiprocate_VER = 1.7
resiprocate_REV = 9367
resiprocate_SPEC = $(SRC)/$(PROJ)/resiprocate.spec
resiprocate_SRPM = resiprocate-$(resiprocate_VER)-$(resiprocate_REV).src.rpm
resiprocate_SOURCES = \
	resiprocate-$(resiprocate_VER)r$(resiprocate_REV).tar.gz

# targets not defined, nothing to do
resiprocate.autoreconf resiprocate.configure resiprocate.dist :;
