AC_ARG_ENABLE([submodule_init],
[  --disable-submodule-init    Initialize Git Submodules],
[case "${enableval}" in
        yes) submodule_init_flag=true ;;
        no)  submodule_init_flag=false ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --disable-submodule-init]) ;;
esac],[submodule_init_flag=true])


SIPX_EDGE_INCDIR=$includedir
SIPX_EDGE_LIBDIR=$libdir
SIPX_EDGE_LIBEXECDIR=$libexecdir
SIPX_EDGE_BINDIR=$bindir
SIPX_EDGE_CONFDIR=$sysconfdir/sipxpbx/edge
SIPX_EDGE_DATADIR=$datadir/sipxpbx
SIPX_EDGE_DOCDIR=$datadir/doc/sipxpbx
SIPX_EDGE_VARDIR=$localstatedir/run/sipxpbx
SIPX_EDGE_TMPDIR=$localstatedir/run/sipxpbx/tmp
SIPX_EDGE_DBDIR=$localstatedir/run/sipxpbx/data
SIPX_EDGE_LOGDIR=$localstatedir/log/sipxpbx
SIPX_EDGE_RUNDIR=$localstatedir/run/sipxpbx
SIPX_EDGE_VARLIB=$localstatedir/lib/sipxpbx
SIPX_EDGE_PREFIX=$prefix
SIPX_EDGE_LIBS_DIR=/opt/sipxpbx/edge
SIPX_EDGE_CWD=`pwd`

FREESWITCH_INSTALLDIR=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
FREESWITCH_BUILDDIR=${SIPX_EDGE_CWD}/libs/freeswitch

MONIT_INSTALLDIR=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
MONIT_BUILDDIR=${SIPX_EDGE_CWD}/libs/monit

CSV_PARSER_INSTALLDIR=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
CSV_PARSER_BUILDDIR=${SIPX_EDGE_CWD}/libs/csv_parser

SIPX_EDGE_BUILD_DIR=${SIPX_EDGE_CWD}
AC_SUBST(SIPX_EDGE_BUILD_DIR)

JSON_RPC_INSTALLDIR=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
JSON_RPC_BUILDDIR=${SIPX_EDGE_CWD}/libs/libjson-rpc-cpp


AC_DEFUN([SFAC_RESIPROCATE_CONFIGURE],
[
  RESIPROCATE_LIBS="-lcares -ldb_cxx "
  RESIPROCATE_LIBS+=" -lrepro "
  RESIPROCATE_LIBS+=" -dum "
  RESIPROCATE_LIBS+=" -lresip "
  RESIPROCATE_LIBS+=" -lrutil "
  AC_SUBST(RESIPROCATE_LIBS)
])

AC_DEFUN([SFAC_FREESWITCH_CONFIGURE],
[
    echo " - Checking changes in libs/freeswitch"

    #
    # Initialize and configure FreeSWITCH submodule
    #

    
    cd ${srcdir}
    SRC_DIR=`pwd`
    FREESWITCH_SRCDIR=${SRC_DIR}/libs/freeswitch
    

    cd ${SIPX_EDGE_CWD}
    checksum_value_new=`find ${FREESWITCH_SRCDIR} -type f -name "*.*" -exec md5sum {} + | awk '{print $1}' | sort | md5sum`

    if [test -f ./.freeswitch.checksum]; then
        checksum_value_old=`cat ./.freeswitch.checksum`
    fi

    if [test x$submodule_init_flag = xtrue]; then         
        #
        # Copy freeswitch sources to build directory.  Freeswitch only supports configure inside top_srcdir
        #
        if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
            rm -rf ${FREESWITCH_INSTALLDIR}
            rm -rf ${FREESWITCH_BUILDDIR}
            mkdir -p ${FREESWITCH_BUILDDIR}
            cp -rpP ${FREESWITCH_SRCDIR}/* ${FREESWITCH_BUILDDIR}
        fi

    fi

    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        cd ${FREESWITCH_BUILDDIR}
        if [! test -f Makefile.in]; then
            if [! ./bootstrap.sh]; then
                AC_MSG_ERROR([Unable to bootstrap submodule libs/freeswitch])
            fi
        fi

        if [test -d ${FREESWITCH_BUILDDIR}]; then
            cd ${FREESWITCH_BUILDDIR}
            cp ${srcdir}/libs/freeswitch_modules.conf.in  ${FREESWITCH_BUILDDIR}/build/modules.conf.in
            if [! ./configure -C --prefix=${FREESWITCH_INSTALLDIR}]; then
                AC_MSG_ERROR([Unable to configure submodule libs/freeswitch])
            fi
        fi
    fi

    cd ${SIPX_EDGE_CWD}
    rm -f ./.freeswitch.checksum
    echo "$checksum_value_new" > ./.freeswitch.checksum

    FREESWITCH_BINARY=${prefix}/opt/ossapp/freeswitch/bin/freeswitch
    AC_SUBST(_FREESWITCH_BINARY)

    cd ${SIPX_EDGE_CWD}
])


AC_DEFUN([SFAC_MONIT_CONFIGURE],
[
    echo " - Checking changes in libs/monit"

    PROJECT_DIR=libs/monit
    CHECKSUM_FILE=.monit.checksum
    SIPX_EDGE_CWD=`pwd`

    cd ${srcdir}
    SRC_DIR=`pwd`
    MONIT_ARCHIVE=${SRC_DIR}/libs/monit/monit.tar.gz

    cd ${SIPX_EDGE_CWD}
    checksum_value_new=`find ${SRC_DIR}/libs/monit/ -type f -name "monit.tar.gz" -exec md5sum {} + | awk '{print $1}' | sort | md5sum`
    if [test -f ./${CHECKSUM_FILE}]; then
        checksum_value_old=`cat ./${CHECKSUM_FILE}`
    fi

    if [test x$submodule_init_flag = xtrue]; then
        if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
            rm -rf ${MONIT_BUILDDIR}
            mkdir -p ${SIPX_EDGE_CWD}/libs
            cd ${SIPX_EDGE_CWD}/libs
            tar -xvf ${MONIT_ARCHIVE}
        fi
    fi

    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        if [test -d ${MONIT_BUILDDIR}]; then
            cd ${MONIT_BUILDDIR}
            if [! ./bootstrap ]; then
                AC_MSG_ERROR([Unable to bootstrap submodule ${PROJECT_DIR}])
            fi

            if [! ./configure --without-pam --enable-optimized --prefix=${MONIT_INSTALLDIR} ]; then
                AC_MSG_ERROR([Unable to configure submodule ${PROJECT_DIR}])
            fi
        fi
    fi

    cd ${SIPX_EDGE_CWD}
    rm -f ./${CHECKSUM_FILE}
    echo "$checksum_value_new" > ./${CHECKSUM_FILE}

    MONIT_PREFIX=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
    AC_SUBST(MONIT_PREFIX)

    cd ${SIPX_EDGE_CWD}
])


AC_DEFUN([SFAC_CSV_PARSER_CONFIGURE],
[
    echo " - Checking changes in libs/libcsv_parser_cpp"

    PROJECT_DIR=libs/csv_parser
    CHECKSUM_FILE=.csv_parser.checksum
    SIPX_EDGE_CWD=`pwd`

    cd ${srcdir}
    SRC_DIR=`pwd`
    CSV_PARSER_SRC_DIR=${SRC_DIR}/libs/csv_parser

    cd ${SIPX_EDGE_CWD}
    checksum_value_new=`find ${SRC_DIR}/libs/csv_parser/ -type f -name "*.*" -exec md5sum {} + | awk '{print $1}' | sort | md5sum`
    if [test -f ./${CHECKSUM_FILE}]; then
           checksum_value_old=`cat ./${CHECKSUM_FILE}`
    fi

    if [test x$submodule_init_flag = xtrue]; then
        if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
            rm -rf ${CSV_PARSER_BUILDDIR}
            mkdir -p ${CSV_PARSER_BUILDDIR}
            cp -rpP ${CSV_PARSER_SRC_DIR}/* ${CSV_PARSER_BUILDDIR}
        fi
    fi

    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        if [test -d ${CSV_PARSER_BUILDDIR}]; then
            cd ${CSV_PARSER_BUILDDIR}
            if [! autoreconf -if ]; then
                AC_MSG_ERROR([Unable to bootstrap submodule ${PROJECT_DIR}])
            fi

            if [! ./configure --prefix=${CSV_PARSER_INSTALLDIR} ]; then
                AC_MSG_ERROR([Unable to configure submodule ${PROJECT_DIR}])
            fi
        fi
    fi

    cd ${SIPX_EDGE_CWD}
    rm -f ./${CHECKSUM_FILE}
    echo "$checksum_value_new" > ./${CHECKSUM_FILE}

    CSV_PARSER_PREFIX=${SIPX_EDGE_PREFIX}/${SIPX_EDGE_LIBS_DIR}
    AC_SUBST(CSV_PARSER_PREFIX)

    CSV_PARSER_LIBS="${CSV_PARSER_BUILDDIR}/.libs/libcsv_parser.a"
    AC_SUBST(CSV_PARSER_LIBS)

    cd ${SIPX_EDGE_CWD}
])


AC_DEFUN([SFAC_JSON_RPC_CONFIGURE],
[
  echo " - Checking changes in libs/libjson-rpc-cpp"

  CHECKSUM_FILE=.jsonrpc.checksum

  cd ${srcdir}
  JSON_RPC_SRCDIR=`pwd`/libs/libjson-rpc-cpp

  cd ${SIPX_EDGE_CWD}
  checksum_value_new=`find ${JSON_RPC_SRCDIR} -type f -name "*.*" -exec md5sum {} + | awk '{print $1}' | sort | md5sum`
  if [test -f ./${CHECKSUM_FILE}]; then
    checksum_value_old=`cat ./${CHECKSUM_FILE}`
  fi

  if [test x$submodule_init_flag = xtrue]; then
    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        rm -rf ${JSON_RPC_BUILDDIR}
        mkdir -p ${JSON_RPC_BUILDDIR}
        cp -rpP ${JSON_RPC_SRCDIR}/* ${JSON_RPC_BUILDDIR}
    fi
  fi

  if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
    cd ${JSON_RPC_BUILDDIR}
    autoreconf -if
    if [! ./configure --prefix=${JSON_RPC_INSTALLDIR}]; then
      AC_MSG_ERROR([Unable to configure Json RPC])
    fi
  fi

  AC_CHECK_HEADERS([curl/curl.h], [], [SF_MISSING_DEP("Curl library not found.  Install libcurl-devel.")])
  AC_CHECK_LIB(curl, main, [CURL_LIBS="-lcurl"], [SF_MISSING_DEP("Curl library not found.  Install libcurl-devel.")])
  AC_SUBST(CURL_LIBS)

  JSON_RPC_LIBS=" ${JSON_RPC_BUILDDIR}/src/.libs/libjsonrpccpp.a $CURL_LIBS"

  cd ${SIPX_EDGE_CWD}
  rm -f ./${CHECKSUM_FILE}
  echo "$checksum_value_new" > ./${CHECKSUM_FILE}


  AC_SUBST(JSON_RPC_LIBS)
  cd ${SIPX_EDGE_CWD}
])
