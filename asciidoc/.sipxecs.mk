asciidoc_SRPM = asciidoc-8.4.5-1.src.rpm
asciidoc_SPEC = $(SRC)/$(PROJ)/asciidoc.spec
asciidoc_SOURCES = asciidoc-8.4.5.tar.bz2 \
  $(SRC)/$(PROJ)/a2x-missing-package-msg.diff \
  $(SRC)/$(PROJ)/asciidoc-8.2.6-no-safe-check.diff \
  $(SRC)/$(PROJ)/asciidoc.changes \
  $(SRC)/$(PROJ)/asciidoc-ignore-deprecation.diff \
  $(SRC)/$(PROJ)/asciidoc-vim-fix.diff \


# targets not defined, nothing to do
asciidoc.autoreconf asciidoc.configure asciidoc.dist:;

