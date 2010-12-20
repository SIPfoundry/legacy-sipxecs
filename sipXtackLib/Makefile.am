include config/project.am

if EXAMPLES
EXAMPLES_OPT = examples
endif

SUBDIRS = \
	. \
	include \
	src \
	meta \
	examples \
	dialogwatch \
	siptest \
	$(EXAMPLES_OPT)

@PACKAGE@_EXTRA_DIST = \
    @PACKAGE@-rpmlintrc
