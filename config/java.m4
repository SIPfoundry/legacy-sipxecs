##
## AC macros for packages using java
##


# SFAC_VAR_APPEND(VARIABLE, VALUE)
# ---------------------------
# Set the VALUE of the shell VARIABLE.
# If variable is already set appends the VALUE to the variable.
# Colon (:) is a hardcoded separator.
AC_DEFUN([SFAC_VAR_APPEND],
[
	AS_VAR_SET_IF([$1],
		[AS_VAR_SET([$1],["$$1:$2"])],
		[AS_VAR_SET([$1],[$2])]
	)
])

# ============ J P A C K A G E ==============
AC_DEFUN([CHECK_JPACKAGE],
[
  AC_ARG_VAR(JAVA_LIBDIR, [JAR location usually /usr/share/java])
  AC_ARG_VAR(CLASSPATH_RUN, [Cumulative classpath to be used during standard run])
  AC_ARG_VAR(CLASSPATH_BUILD, [Cumulative classpath to be used during builds])

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

  AS_VAR_SET_IF([found_jpackage],
		[AC_MSG_RESULT([    jar directory found in $JAVA_LIBDIR])],
		[AC_MSG_ERROR([Cannot find system jar directory.  Try to install jpackage-utils package.])]
	)
])


# ====== J P A C K A G E   U T I L S ========
AC_DEFUN([CHECK_JPACKAGE_UTILS],
[
  AC_REQUIRE([CHECK_JPACKAGE])

  AC_ARG_VAR(JPACKAGE_FIND_JAR, [japackage utility to check for jars])
  AC_PATH_PROG([JPACKAGE_FIND_JAR],[find-jar])
  AS_VAR_SET_IF([JPACKAGE_FIND_JAR], , [AC_MSG_ERROR([cannot find find-jar utility])])

  AC_ARG_VAR(JPACKAGE_FIND_JAR, [japackage utility to build classpath])
  AC_PATH_PROG([JPACKAGE_CLASSPATH_BUILD],[build-classpath])
  AS_VAR_SET_IF([JPACKAGE_CLASSPATH_BUILD], , [AC_MSG_ERROR([cannot find build-classpath utility])])
])

# =========== C H E C K   J A R =============
#
# CHECK_JAR(VARIABLE, JAR_TO_FIND, [CLASSPATH_VARIABLE])
# finds JAR_TO_FIND by name and sets VARIABLE to its location
# appends VARIABLE to CLASSPATH_RUN and CLASSPATH_BUILD
# unless CLASSPATH_VARIABLE is specified, in which case only appends to CLASSPATH_VARIABLE
#
AC_DEFUN([CHECK_JAR],
[
	AC_REQUIRE([CHECK_JPACKAGE_UTILS])
	AC_ARG_VAR([$1], [$2 location])

	jar=`$JPACKAGE_FIND_JAR $2 2> /dev/null`
	AC_CHECK_FILE(
		[$jar],
		[
			AS_VAR_SET([$1],[$jar])
			# append to both classpaths, unless there is a specific one to be set
			if test x$3 != x; then
				SFAC_VAR_APPEND([$3],[$jar])
			else
				SFAC_VAR_APPEND([CLASSPATH_RUN],[$jar])
				SFAC_VAR_APPEND([CLASSPATH_BUILD],[$jar])
			fi
		],
		[AC_MSG_ERROR([cannot find jar $2])]
	)
])

# ================= J A R S ==================
# macros for common jars
AC_DEFUN([CHECK_JUNIT],[CHECK_JAR([JUNIT_JAR],[junit],[CLASSPATH_BUILD])])

AC_DEFUN([CHECK_JUNIT4],[CHECK_JAR([JUNIT4_JAR],[junit4],[CLASSPATH_BUILD])])

