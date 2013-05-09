AC_DEFUN([CHECK_DART_SDK],
[
   DART_HOME=
   candidates="/opt/dart/dart-sdk /opt/dart-sdk ~/dart-sdk"
   AC_MSG_CHECKING([dart-sdk])
   for candidate in $candidates; do
     if test -f $candidate/bin/dart2js ; then
       DART_HOME=$candidate
       break
     fi
   done

   if test x$DART_HOME == x; then
     SF_MISSING_DEP([Cannot find dart sdk])
   else
     AC_MSG_RESULT(${DART_HOME})
   fi

   AC_SUBST(DART_HOME)
])