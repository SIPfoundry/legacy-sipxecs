
# ============ J D K  =======================
AC_DEFUN([CHECK_JDK],
[
    AC_ARG_VAR(JAVA_HOME, [Java Development Kit])

    TRY_JAVA_HOME=`ls -dr /usr/java/* 2> /dev/null | head -n 1`
    for dir in $JAVA_HOME $JDK_HOME /usr/lib/jvm/java /usr/lib64/jvm/java /usr/local/jdk /usr/local/java $TRY_JAVA_HOME; do
        AC_CHECK_FILE([$dir/lib/dt.jar],[jar=$dir/lib/dt.jar])
        if test x$jar != x; then
            found_jdk="yes";
            JAVA_HOME=$dir
            break;
        fi
    done

    if test x_$found_jdk != x_yes; then
        AC_MSG_ERROR([Cannot find dt.jar in expected location. You may try setting the JAVA_HOME environment variable if you haven't already done so])
    fi

    AC_SUBST(JAVA, [$JAVA_HOME/jre/bin/java])

    AC_ARG_VAR(JAVAC_OPTIMIZED, [Java compiler option for faster performance. Default is on])
    test -z $JAVAC_OPTIMIZED && JAVAC_OPTIMIZED=on

    AC_ARG_VAR(JAVAC_DEBUG, [Java compiler option to reduce code size. Default is off])
    test -z $JAVAC_DEBUG && JAVAC_DEBUG=off
])


# ============ J N I =======================
AC_DEFUN([CHECK_JNI],
[
   AC_REQUIRE([CHECK_JDK])

   JAVA_HOME_INCL=$JAVA_HOME/include
   AC_CHECK_FILE([$JAVA_HOME_INCL/jni.h], 
     [CPPFLAGS="-I$JAVA_HOME_INCL -I$JAVA_HOME_INCL/linux"],
     AC_MSG_ERROR([Cannot find or validate header file $JAVA_HOME_INCL/jni.h])
   )
])

