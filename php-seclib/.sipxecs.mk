php-seclib_VER = 0.2.1
php-seclib_SVN = a
php-seclib_REL = 0.$(php-seclib_SVN).1

php-seclib_SRPM = php-seclib-$(php-seclib_VER)-$(php-seclib_REL).src.rpm
php-seclib_SPEC = $(SRC)/$(PROJ)/php-seclib.spec

php-seclib_SOURCES = \
	$(SRC)/$(PROJ)/includes.patch \
    phpseclib$(php-seclib_VER)$(php-seclib_SVN).zip

# targets not defined, nothing to do
php-seclib.autoreconf php-seclib.configure php-seclib.dist :;
