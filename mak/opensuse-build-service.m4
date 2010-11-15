AC_ARG_ENABLE(obs, [Enable targets for using the OpenSUSE build service],
[
  AC_ARG_VAR(OBS_PROJECT, [sipXecs project on opensuse builder service])
  if test -z "${OBS_PROJECT}"; then
    OBS_PROJECT=home:sipfoundry:main
  fi

  AC_ARG_VAR(OBS_DIR, [Checkout of sipXecs project from opensuse builder service])
  if test -z "${OBS_DIR}"; then
    OBS_DIR=obs
  fi

  AC_CONFIG_FILES([mak/15-opensuse-build-service.mk])
])
