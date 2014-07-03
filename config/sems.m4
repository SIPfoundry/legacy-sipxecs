AC_DEFUN([SFAC_SEMS_CONFIGURE],
[
    #
    # Initialize and configure sems submodule
    #
    CURRENT_DIR=`pwd`
    SEMS_BUILDDIR=${CURRENT_DIR}/applications/sems
    SEMS_INSTALLDIR=${prefix}

    cd ${srcdir}
    SRC_DIR=`pwd`
    SEMS_SRCDIR=${SRC_DIR}/applications/sems/

    SEMS_CONFIG_FLAGS=" -DSEMS_CFG_PREFIX=${SEMS_INSTALLDIR}/etc/sipxpbx/edge/sems "
    SEMS_CONFIG_FLAGS+=" -DSEMS_AUDIO_PREFIX=${SEMS_INSTALLDIR}/lib "
    SEMS_CONFIG_FLAGS+=" -DSEMS_EXEC_PREFIX=${SEMS_INSTALLDIR} "
    SEMS_CONFIG_FLAGS+=" -DSEMS_DOC_PREFIX=${SEMS_INSTALLDIR}/share/sems/doc "

    mkdir -p ${SEMS_BUILDDIR}

    if [test -d ${SEMS_BUILDDIR}]; then
        cd ${SEMS_BUILDDIR}
        if [! cmake ${SEMS_CONFIG_FLAGS} ${SEMS_SRCDIR}]; then
            AC_MSG_ERROR([Unable to configure submodule src/sems])
        fi
    fi

    cd ${CURRENT_DIR}
])