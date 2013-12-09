

AC_DEFUN([SFAC_RESIP_DEVEL],
[
    RESIP_LIBS="$RESIP_LIBS -ldb_cxx"
    RESIP_LIBS="$RESIP_LIBS -lcares"
    RESIP_LIBS="$RESIP_LIBS -lrutil"
    RESIP_LIBS="$RESIP_LIBS -lresip"
    RESIP_LIBS="$RESIP_LIBS -ldum"
    RESIP_LIBS="$RESIP_LIBS -lrepro"
    AC_SUBST(RESIP_LIBS)
])


AC_DEFUN([SFAC_FREESWITCH_DEVEL],
[
    FREESWITCH_BUILDDIR=`pwd`/src/freeswitch
    FREESWITCH_BASEDIR=${prefix}/opt/sipx/sipXrtcbridge
    FREESWITCH_INCDIR=$FREESWITCH_BASEDIR/include
    FREESWITCH_LIBDIR=$FREESWITCH_BASEDIR/lib
    FREESWITCH_MODDIR=$FREESWITCH_BASEDIR/mod
    FREESWITCH_PATH=$FREESWITCH_BASEDIR/bin/freeswitch

    AC_SUBST(FREESWITCH_INCDIR)
    AC_SUBST(FREESWITCH_LIBDIR)
    AC_SUBST(FREESWITCH_MODDIR)

    CFLAGS="$CFLAGS -I$FREESWITCH_INCDIR"
    CXXFLAGS="$CXXFLAGS -DFREESWITCH_LIBDIR=\\\"$FREESWITCH_LIBDIR\\\""
    CXXFLAGS="$CXXFLAGS -DFREESWITCH_MODDIR=\\\"$FREESWITCH_MODDIR\\\""
    CXXFLAGS="$CXXFLAGS -DFREESWITCH_PATH=\\\"$FREESWITCH_PATH\\\""
    CXXFLAGS="$CXXFLAGS -I$FREESWITCH_BUILDDIR/libs/esl/src/include" 

    FREESWITCH_LIBS="$FREESWITCH_LIBS $FREESWITCH_BUILDDIR/libs/esl/libesl.a"

    AC_SUBST(FREESWITCH_LIBS)
]) 


AC_DEFUN([SFAC_SUBMODULE_INIT_FREESWITCH],
[
    AC_ARG_ENABLE([submodule_init],
    [  --disable-submodule-init    Initialize Git Submodules],
    [case "${enableval}" in
            yes) submodule_init_flag=false ;;
            no)  submodule_init_flag=true ;;
            *) AC_MSG_ERROR([bad value ${enableval} for --disable-submodule-init]) ;;
    esac],[submodule_init_flag=true])

    #
    # Initialize and configure FreeSWITCH submodule
    #
    current_directory=`pwd`
    freeswitch_srcdir=${srcdir}/src/freeswitch
    freeswitch_builddir=${current_directory}/src/freeswitch

    if [test x$submodule_init_flag = xtrue]; then
        cd ${srcdir}/../
        if [! test -f sipXrtcbridge/src/freeswitch/Makefile.am]; then
            if [! git submodule init sipXrtcbridge/src/freeswitch]; then
                AC_MSG_ERROR([Unable to initialize git submodule sipXrtcbridge/src/freeswitch])
            fi
            if [! git submodule update sipXrtcbridge/src/freeswitch]; then
                AC_MSG_ERROR([Unable to update git submodule sipXrtcbridge/src/freeswitch])
            fi
        fi

        cd $freeswitch_srcdir
        if [! git checkout v1.4.beta]; then
            AC_MSG_ERROR([Unable to checkout v1.4.beta branch for git submodule sipXrtcbridge/src/freeswitch])
        fi
        
        cd ${current_directory}
    fi


    
    #
    # Copy freeswitch sources to build directory.  Freeswitch only supports configure inside top_srcdir
    #
    cd ${current_directory}
    checksum_value_new=`find ${freeswitch_srcdir} -type f -name "*.*" -exec md5sum {} + | awk '{print $1}' | sort | md5sum`
    
    if [test -f ./.freeswitch.checksum]; then
        checksum_value_old=`cat ./.freeswitch.checksum`
    fi

    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        echo ">>>>>>>>>>>>>>> ${checksum_value_new} NOT EQUAL  ${checksum_value_old} <<<<<<<<<<<<<<<<<"
    else
        echo ">>>>>>>>>>>>>>> ${checksum_value_new} EQUALS  ${checksum_value_old} <<<<<<<<<<<<<<<<<"
    fi

    if [[ "x${checksum_value_new}" != "x${checksum_value_old}" ]]; then
        rm -rf ${freeswitch_builddir}
        mkdir -p src
        cp -rpP ${freeswitch_srcdir} src

        cd ${freeswitch_builddir}
        if [! test -f Makefile.in]; then
            if [! ./bootstrap.sh]; then
                AC_MSG_ERROR([Unable to bootstrap submodule sipXrtcbridge/src/freeswitch])	 
            fi
        fi

        if [test -d ${freeswitch_builddir}]; then
            #rm -rf ${prefix}/opt/sipx/sipXrtcbridge
            cd ${freeswitch_builddir}
            cp ${srcdir}/src/freeswitch_modules.conf.in  ${freeswitch_builddir}/build/modules.conf.in
            if [! ./configure -C --prefix=${prefix}/opt/sipx/sipXrtcbridge]; then
                AC_MSG_ERROR([Unable to configure submodule sipXrtcbridge/src/freeswitch])
            fi
        fi
    fi

    cd ${current_directory}
    rm -f ./.freeswitch.checksum
    echo "$checksum_value_new" > ./.freeswitch.checksum
])


AC_DEFUN([SFAC_SUBMODULE_INIT_JSONRPC],
[
    AC_ARG_ENABLE([submodule_init],
    [  --disable-submodule-init    Initialize Git Submodules],
    [case "${enableval}" in
            yes) submodule_init_flag=false ;;
            no)  submodule_init_flag=true ;;
            *) AC_MSG_ERROR([bad value ${enableval} for --disable-submodule-init]) ;;
    esac],[submodule_init_flag=true])

    #
    # Initialize and configure libjsonrpccpp submodule
    #
    current_directory=`pwd`
    libjsonrpccpp_srcdir=${srcdir}/src/libjsonrpccpp
    libjsonrpccpp_builddir=${current_directory}/src/libjsonrpccpp

    if [test x$submodule_init_flag = xtrue]; then
        cd ${srcdir}/../
        if [! test -f sipXrtcbridge/src/libjsonrpccpp/Makefile.am]; then
            if [! git submodule init sipXrtcbridge/src/libjsonrpccpp]; then
                AC_MSG_ERROR([Unable to initialize git submodule sipXrtcbridge/src/libjsonrpccpp])
            fi
            if [! git submodule update sipXrtcbridge/src/libjsonrpccpp]; then
                AC_MSG_ERROR([Unable to update git submodule sipXrtcbridge/src/libjsonrpccpp])
            fi
        fi

        cd $libjsonrpccpp_srcdir
        if [! git checkout master]; then
            AC_MSG_ERROR([Unable to checkout master branch for git submodule sipXrtcbridge/src/libjsonrpccpp])
        fi
        
        cd ${current_directory}
    fi
])





