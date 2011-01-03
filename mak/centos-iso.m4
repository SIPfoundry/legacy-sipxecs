dnl Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
dnl Licensed to the User under the LGPL license.
dnl
AC_ARG_ENABLE(centos-iso, [--enable-centos-iso Build sipXecs or custom CD],
[
  AC_ARG_VAR(ISO_DIR, [Directory containing your CentOS CD disk 1 ISO])
  if test "x$ISO_DIR" == "x"; then
    ISO_DIR=~/Downloads
  fi

  if [ ! test -d $ISO_DIR ]; then
    AC_MSG_ERROR(Directory $ISO_DIR is missing)
  fi
  
  if [ ! test -d $RPM_DIST_DIR ]; then
    AC_MSG_ERROR(Directory $REPO_DIR is missing)
  fi

  AC_ARG_VAR(OEM_DIR, [Optional: Directory containing your customizations])
  if test "x$OEM_DIR" != "x"; then
    OEM_MAK="$OEM_DIR/iso.mak"
    AC_SUBST(OEM_MAK)
    OEM_M4_OPTS="-I $OEM_DIR"
    AC_SUBST(OEM_M4_OPTS)
  fi

  AC_ARG_WITH([distdir],
    AC_HELP_STRING([--with-distdir=directory], 
      [Directory to output distribution output ISO files]),
    [DIST_DIR=${withval}],
    [DIST_DIR=.]
  )
  AC_SUBST([DIST_DIR])

  AC_PATH_PROG(CREATEREPO, createrepo)
  if test "x$CREATEREPO" == "x"; then
    AC_MSG_ERROR(createrepo needs to be installed)
  fi

  AC_CONFIG_FILES([
     mak/35-centos-iso.mk 
     $OEM_MAK])
])
