# There are two systems used by the 2.0.* releases for specifying the
# version number.  One puts the entire version number in #define
# AP_SERVER_BASEREVISION.  The other splits the three components of
# the version number among AP_SERVER_MAJORVERSION,
# AP_SERVER_MINORVERSION, and AP_SERVER_PATCHLEVEL.
/#define AP_SERVER_MAJORVERSION/ { a1 = substr($3,2,length($3)-2) }
/#define AP_SERVER_MINORVERSION/ { a2 = substr($3,2,length($3)-2) }
/#define AP_SERVER_PATCHLEVEL/   { a3 = substr($3,2,length($3)-2) }
/#define AP_SERVER_BASEREVISION/ { b = substr($3,2,length($3)-2) }
END { print ((a1 != "") ? a1"."a2"."a3 : b) }
