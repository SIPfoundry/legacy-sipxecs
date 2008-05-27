##
## AC macros for packages using java
##

# ============ J P A C K A G E ==============
AC_DEFUN([CHECK_JPACKAGE],
[
    AC_ARG_VAR(JAVA_LIBDIR, [JAR location usually /usr/share/java])

    for dir in /etc/java ; do
        AC_CHECK_FILE([$dir/java.conf],[java_conf=$dir/java.conf])
        if test x$java_conf != x; then
            # sets JAVA_LIBDIR
            source $java_conf
            if test x$JAVA_LIBDIR != x; then
                found_jpackage="yes";
                break;
            fi
        fi
    done

    if test x_$found_jpackage = x_yes; then
        AC_MSG_RESULT([    jar directory found in $JAVA_LIBDIR])
    else
        AC_MSG_ERROR([Cannot system jar directory. Try to install jpackage-utils package.])
    fi
])


# ====== J P A C K A G E   U T I L S ========
AC_DEFUN([CHECK_JPACKAGE_UTILS],
[
    AC_REQUIRE([CHECK_JPACKAGE])

    AC_ARG_VAR(JPACKAGE_FIND_JAR, [japackage utility to check for jars])
    AC_PATH_PROG([JPACKAGE_FIND_JAR],[find-jar])
    if test x$JPACKAGE_FIND_JAR == x; then
        AC_MSG_ERROR([cannot find find-jar utility])
    fi

    AC_ARG_VAR(JPACKAGE_FIND_JAR, [japackage utility to build classpath])
    AC_PATH_PROG([JPACKAGE_BUILD_CLASSPATH],[build-classpath])
    if test x$JPACKAGE_BUILD_CLASSPATH == x; then
        AC_MSG_ERROR([cannot find build-classpath utility])
    fi
])

# =========== C H E C K   J A R =============
#
# CHECK_JAR(VARIABLE, JAR_TO_FIND)
# finds jar by name and sets environment variable
#
AC_DEFUN([CHECK_JAR],
[
    AC_REQUIRE([CHECK_JPACKAGE_UTILS])
		AC_ARG_VAR([$1], [$2 location])

		jar=`$JPACKAGE_FIND_JAR $2 2> /dev/null`
		AC_CHECK_FILE(
				[$jar],
				[AS_VAR_SET([$1],[$jar])],
				[AC_MSG_ERROR([cannot find jar $2])]
		)
])

# ================= J A R S ==================
# macros for common jars
AC_DEFUN([CHECK_JUNIT],[CHECK_JAR([JUNIT_JAR],[junit])])

AC_DEFUN([CHECK_JUNIT4],[CHECK_JAR([JUNIT4_JAR],[junit4])])

AC_DEFUN([CHECK_LOG4J],[CHECK_JAR([LOG4J_JAR],[log4j])])

# ============ J A I N - S I P ==============
AC_DEFUN([CHECK_JAIN_SIP],
[
		CHECK_JAR([JAIN_SIP_API_JAR],[jain-sip/jain-sip-api])
		CHECK_JAR([JAIN_SIP_RI_JAR],[jain-sip/jain-sip-ri])
		CHECK_JAR([JAIN_SIP_SDP_JAR],[jain-sip/sip-sdp])
])


