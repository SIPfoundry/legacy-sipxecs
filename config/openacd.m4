AC_DEFUN([SIPX_OPENACD],
[
  AC_ARG_WITH(openacd, [--with-openacd={yes,no,openacd-location} Default is yes and will search 
                        for openacd in standard locations], 
openacd_prefix="$withval",  
openacd_prefix="yes")

  if test "$openacd_prefix" == "yes" ; then
    openacd_prefix="${prefix}/OpenACD /opt/OpenACD"
  fi

  if test "$openacd_prefix" != "no"; then
    AC_MSG_CHECKING([for OpenACD])
    for d in $openacd_prefix ; do
      if test -d $d ; then
        OPENACD_DIR=$d
        AC_MSG_RESULT([yes - $d])
        openacd_found=yes
        break;
      fi
    done

    if test -z "$openacd_found" ; then
      AC_MSG_RESULT([no])
    fi
    AC_SUBST([OPENACD_DIR])

    AC_MSG_CHECKING([for OpenACD include files])
    openacd_inc="include lib/OpenACD-*/include"
    for d in $openacd_inc; do
      de=`readlink -f $OPENACD_DIR/$d`
      if test -f $de/log.hrl; then
        OPENACD_ERLCFLAGS="-I $de"
	openacdinc_found=yes
        AC_MSG_RESULT([yes $de])
	break;
      fi
    done

    if test -z "$openacdinc_found" ; then
      AC_MSG_RESULT([no])
    fi
    AC_SUBST([OPENACD_ERLCFLAGS])
  fi
])
