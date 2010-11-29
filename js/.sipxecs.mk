js_SRPM = js-1.70-8$(RPM_DIST).src.rpm
js_SPEC = $(SRC)/$(PROJ)/js.spec
# sources obtained from https://build.opensuse.org/project/show?project=home%3Amozillamessaging%3Araindrop
js_SOURCES = js-1.7.0.tar.gz \
  $(SRC)/$(PROJ)/js-1.5-va_copy.patch \
  $(SRC)/$(PROJ)/js-1.60-ncurses.patch \
  $(SRC)/$(PROJ)/js-1.7.0-make.patch \
  $(SRC)/$(PROJ)/js-1.7.0-threadsafe.patch \
  $(SRC)/$(PROJ)/js-ldflags.patch \
  $(SRC)/$(PROJ)/js-perlconnect.patch \
  $(SRC)/$(PROJ)/js-shlib.patch

# targets not defined, nothing to do
js.autoreconf js.configure js.dist:;
