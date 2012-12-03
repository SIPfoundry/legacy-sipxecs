# This script does not find much of anything
#    m4_include([config/ax_check_java_home.m4])
# as a replace include either one of these
#   config/check_jdk.m4
#   config/general.m4
# 
m4_include([config/ax_check_java_plugin.m4])
m4_include([config/ax_java_check_class.m4])
m4_include([config/ax_java_options.m4])
m4_include([config/ax_prog_java_cc.m4])
# force javac but unknown if any other compiler works
JAVAC=javac
m4_include([config/ax_prog_javac.m4])
m4_include([config/ax_prog_javac_works.m4])
m4_include([config/ax_prog_javadoc.m4])
m4_include([config/ax_prog_javah.m4])
m4_include([config/ax_prog_java.m4])
m4_include([config/ax_prog_java_works.m4])
m4_include([config/ax_try_compile_java.m4])
m4_include([config/ax_try_run_javac.m4])

# Find the java-dep program, looking in installation path but you can pass
# in an optional path if you know where it might be
AC_DEFUN([PROG_JAVA_DEP],
[
  if test x$1 == x; then
    eval check_path=${bindir}
    # this will resolve nested variable references, twice plenty
    eval check_path=${check_path}
    eval check_path=${check_path}
  else
    check_path=$1
  fi
  AC_PATH_PROG(JAVA_DEP, java-dep,,${check_path}:$PATH)
])


AC_ARG_ENABLE([debug], 
  AC_HELP_STRING([--enable-debug], [Build project with debug flags. Default is off]))
if test "x$enable_debug" = "xyes"; then
  JAVA_CC_FLAGS='-encoding UTF-8 -g:source,lines,vars'
else
  JAVA_CC_FLAGS='-encoding UTF-8'
fi

