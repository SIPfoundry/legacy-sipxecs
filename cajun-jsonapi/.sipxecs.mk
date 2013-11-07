cajun-jsonapi_VER = 2.0.3
cajun-jsonapi_REL = 1
# Odd tarball name, but reflects Source0: URL defined in cajun-jsonapi.spec
cajun-jsonapi_TAR = $(cajun-jsonapi_VER).tar.gz
cajun-jsonapi_SRPM = cajun-jsonapi-$(cajun-jsonapi_VER)-$(cajun-jsonapi_REL)$(RPM_DIST).src.rpm
cajun-jsonapi_SOURCES = $(cajun-jsonapi_TAR)

cajun-jsonapi.autoconf:;

cajun-jsonapi.dist : $(cajun-jsonapi_TAR);

cajun-jsonapi-$(cajun-jsonapi_VER) :
	git clone --branch $(cajun-jsonapi_VER) git@github.com:cajun-jsonapi/cajun-jsonapi.git $@.tmp
	mv $@.tmp $@

$(cajun-jsonapi_TAR) : cajun-jsonapi-$(cajun-jsonapi_VER)
	tar -czf $@ cajun-jsonapi-$(cajun-jsonapi_VER)
