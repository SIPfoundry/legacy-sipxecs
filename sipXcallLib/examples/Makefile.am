## Process this file with automake to produce Makefile.in

EXTRA_DIST = musicServer CallScript PlaceCall ReceiveCall SendAudio

if CALL_EXAMPLES
example_SUBDIRS = PlaceCall ReceiveCall SendAudio
else
example_SUBDIRS =
endif

SUBDIRS = ${ezphone_SUBDIRS} ${example_SUBDIRS}

DISTCLEANFILES = Makefile.in
