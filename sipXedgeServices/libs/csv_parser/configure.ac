
dnl Mandatory Autoconf Initialization Macro
dnl package_name, version_number, optional bug report email, tarball_name for tar.gz release files
AC_INIT(libcsv_parser++, 1.0.0, israel.ekpo@israelekpo.com, libcsv_parser++-dev)

dnl Defining the AX_EXPAND_PREFIX function
AC_DEFUN([AX_EXPAND_PREFIX],[dnl

  test "x$prefix" = xNONE && prefix=$ac_default_prefix
  
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
])

dnl Makes the value for prefix and exec_prefix available for substitution
AX_EXPAND_PREFIX

dnl Automake initialization. This method is obselete but needed for us to proceed.
AM_INIT_AUTOMAKE(libcsv_parser++, 1.0.0)

dnl Set the default installation prefix to /usr instead of /usr/local
AC_PREFIX_DEFAULT(/usr)

dnl If the user did not specify an installation prefix (using the --prefix option)
dnl guess a value for it by looking for which
dnl If program is found, set the prefix to the parent of the directory containing which
dnl else default the /usr as described above
AC_PREFIX_PROGRAM(which)

dnl Ensures that a recent enough version of autoconf is being used
AC_PREREQ(2.61)

dnl Checking if the compiler supports C++.
AC_LANG(C++)

dnl Check for the C++ compiler.
AC_PROG_CXX

dnl Check for the existence of libtool.
AC_PROG_LIBTOOL

dnl Grab the path where libtool is installed.
AC_CHECK_PROG(libtool_path, libtool, true, false)

dnl Insert the copyright notice
AC_COPYRIGHT([Copyright (C) 2008 - 2009, Israel Ekpo, <israel.ekpo@israelekpo.com>])

dnl This should create the license file.
dnl It also guarantees that the LICENSE.in will be in the tarball.
AC_OUTPUT(LICENSE)

dnl Save the settings into the Makefile.
AC_OUTPUT(Makefile)

dnl Save the changes into the csv_parser-config file
AC_OUTPUT(csv_parser-config)

AC_MSG_NOTICE([

Environment configuration has completed successfully.

You are now ready to run make and make install.

This will install your csv_parser library object code into $prefix/lib and your development header files into $prefix/include/csv_parser

+------------------------------------------------------------------------+
| License:                                                               |
| This software is subject to the new BSD License, available in this     |
| distribution in the file LICENSE.  By continuing this installation     |
| process, you are bound by the terms of this license agreement.         |
| If you do not agree with the terms of this license, you must abort     |
| the installation process at this point.                                |
+------------------------------------------------------------------------+

Thank you for using libcsv_parser++.

])

