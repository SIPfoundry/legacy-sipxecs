#!/bin/bash
#
# Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# ede_build_devuser.sh
#
# See http://wiki.sipfoundry.org/display/xecsdev/Express+Development+Environment+%28EDE%29 for instructions.

# Usage: $0 [-h] [-d] [-r] [-c CODEDIR]
#   -h: Print this usage information and exit.
#   -c CODEDIR: Specify an alternate $CODE directory than the default.
#   -d: Build the dependency RPMs, versus retrieve them from SIPfoundry.
#   -r: Build and install from sipXecs RPMs, versus install directly.
#   -s: Build in sandbox. (Make no changes outside WORKING.)  Incompatible with -r and -d. 

START_DATE=`date`

# These will be created in the current directory.
WORKING_DIR=`pwd`
INSTALL=INSTALL
BUILD=BUILD
RPMBUILD=RPMBUILD
ECLIPSE_WORKSPACE=eclipse-workspace
EDE_BIN=bin_ede
DEP_RPM_TOPDIR=DEP_RPM_TOPDIR
SIPX_RPM_TOPDIR=SIPX_RPM_TOPDIR
CODE=4.2
LINKS=links
DIST=DIST
EDE_LOGS=logs_ede
FULL_PATH_INSTALL=$WORKING_DIR/$INSTALL
FULL_PATH_BUILD=$WORKING_DIR/$BUILD
FULL_PATH_CODE=$WORKING_DIR/$CODE
FULL_PATH_DIST=$WORKING_DIR/$DIST
FULL_PATH_EDE_LOGS=$WORKING_DIR/$EDE_LOGS
FULL_PATH_DEP_RPM_TOPDIR=$WORKING_DIR/$DEP_RPM_TOPDIR
FULL_PATH_SIPX_RPM_TOPDIR=$WORKING_DIR/$SIPX_RPM_TOPDIR
FULL_PATH_ECLIPSE_WORKSPACE=$WORKING_DIR/$ECLIPSE_WORKSPACE
FULL_PATH_EDE_BIN=$WORKING_DIR/$EDE_BIN
EDE_ENV_FILE=env-ede

# Parse the arguments.
USAGE="Usage: $0 [-h] [-d] [-r] [-i] [-s] [-c CODEDIR]"
while getopts hc:rdis OPT; do
   case "$OPT" in
      h) echo $USAGE
         exit 1
         ;;
      d) BUILD_ALL_DEPENDENCIES="yup"
         ;;
      r) BUILD_RPMS="yup"
         ;;
      i) SKIP_LOCAL_SETUP_RUN="yup"
         ;;
      s) SANDBOX_MODE="yup"
         ;;
      c) CODE=$OPTARG
         ;;
      \?)
         echo $USAGE >&2
         exit 1
         ;;
   esac
done

# Ensure we have a code directory.
if [ ! -d $CODE ]; then
  echo "" >&2
  echo "ERROR: Directory './$CODE' does not exist." >&2
  echo "" >&2
  exit 2
fi

if [ "`whoami`" == root ]; then
  echo "" >&2
  echo "ERROR: Do NOT run this script as root (or with sudo.)" >&2
  echo "" >&2
  exit 1
fi

if [ -n "$SANDBOX_MODE" ]; then
   if [ -n "$BUILD_RPMS" ]; then
      echo "" >&2
      echo "ERROR: The -s and -r options are not compatible." >&2
      echo "" >&2
      exit 1
   fi
   if [ -n "$BUILD_ALL_DEPENDENCIES" ]; then
      echo "" >&2
      echo "ERROR: The -s and -d options are not compatible." >&2
      echo "" >&2
      exit 1
   fi
fi

DISTRO_EXIT_ERROR=3
### START - This section is duplicated in both ede_base_root.sh and ede_build_devuser.sh ###

   DISTRO_ID_CentOS5=el5
   DISTRO_ID_Fedora8=fc8
   DISTRO_ID_Fedora10=fc10
   DISTRO_ID_Fedora11=fc11
   CHECK_DISTROS="$DISTRO_ID_CentOS5 $DISTRO_ID_Fedora8 $DISTRO_ID_Fedora10 $DISTRO_ID_Fedora11"
   DISTRO_ID_Unknown=unknown
   function return_uname_distro_id {
      RET_VAL=$DISTRO_ID_Unknown
      for check_distro in $CHECK_DISTROS;
      do
         uname -a | cut -d" " -f3 | grep $check_distro > /dev/null
         if [ $? == 0 ]; then
            RET_VAL=$check_distro
         fi
      done
      echo $RET_VAL
   }
   echo ""
   if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
      echo "CentOS 5!  Best effort support by EDE.  (Not recommended.)"
   elif [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 ]; then
      echo "Fedora 10!  Fully supported by EDE!"
   elif [ $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
      echo "Fedora 11!  Not yet supported by EDE!  (A work in progress.  Not recommended.)"
   elif [ $(return_uname_distro_id) == $DISTRO_ID_Fedora8 ]; then
      echo "Fedora 8 is no longer supported by EDE."
      exit $DISTRO_EXIT_ERROR
   else
      echo -n "Unsupported Linux distribution: "
      uname -a | cut -d" " -f3
      exit $DISTRO_EXIT_ERROR
   fi
   echo ""
   sleep 3

   # Dependencies that are required.  Fedora 10/11 has these available in the standard repository.  For
   # CentOS 5.2, they must be installed from SIPfoundry dependency RPMs.
   BASE_DEPS="xerces-c xerces-c-devel cppunit-devel w3c-libwww w3c-libwww-apps w3c-libwww-devel rrdtool rrdtool-perl rubygems"

   # In Fedora 10/11 nsis and nsis-data are provided by mingw32-nsis, which is avalable from the
   # standard repository.
   if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
      BASE_DEPS="$BASE_DEPS nsis nsis-data"
   else
      BASE_DEPS="$BASE_DEPS mingw32-nsis"
   fi

### END - This section is duplicated in both ede_base_root.sh and ede_build_devuser.sh ###

echo ""
echo "Start   : $START_DATE"
echo ""

function add_ftp_user { # username, password
   sudo /usr/sbin/useradd -d /tftpboot -G `whoami` -s /sbin/nologin -M $1
   echo -e "$2" | sudo passwd --stdin $1
}

function sudo_wget_retry {
   P_OPT=""
   if [ $# == 2 ]; then
      P_OPT="-P $2 "
   fi
   echo "  wget_retry for $P_OPT$1..."
   sudo wget $P_OPT$1
   if [ $? != 0 ]; then
      sudo wget $P_OPT$1
      if [ $? != 0 ]; then
         sudo wget $P_OPT$1
         if [ $? != 0 ]; then
            echo "    FAILED!"
            exit 4
         fi
      fi
   fi
   echo "    SUCCESS."
}

function return_sipxecs_unstable_repo_name {
    if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
       DISTRO_PART=centos
    elif [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 -o $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
       DISTRO_PART=fc
    else
       DISTRO_PART=unsupported-distro
    fi
    echo sipxecs-unstable-$DISTRO_PART
}

NORTEL_INTERNAL_BUILD_HOST=falcon.us.nortel.com
function add_sipxecs_unstable_repo {

   sudo rm -rf /etc/yum.repos.d/$(return_sipxecs_unstable_repo_name).repo
   sudo rm -rf /var/cache/yum/sipxecs-*

   nslookup $NORTEL_INTERNAL_BUILD_HOST | grep "Name:" &> /dev/null
   if [ $? == 0 ]; then
      # Use the build server.
      uname -a | grep el5 &> /dev/null
      if [ $? == 0 ]; then
         DISTRO_PART="CentOS/5"
      else
         DISTRO_PART="FC/8"
      fi
      cat <<EOF >> tmp.repo
[sipxecs-unstable]
name=sipXecs $NORTEL_INTERNAL_BUILD_HOST
baseurl=http://$NORTEL_INTERNAL_BUILD_HOST/scs/branches/4.2/$DISTRO_PART/`uname -i`/current/RPM
gpgcheck=0
enabled=1
EOF
      sudo mv tmp.repo /etc/yum.repos.d/$(return_sipxecs_unstable_repo_name).repo
   else
      # Use sipxecssw.org
      sudo_wget_retry http://sipxecssw.org/pub/sipXecs/$(return_sipxecs_unstable_repo_name).repo /etc/yum.repos.d
      if [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 -o $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
         # SIPfoundry doesn't yet have a dependency repo for 10/11, but the 8 RPMs (except FreeSWITCH) work well.
#
# 22 April 2010 - In fact SIPfoundry doesn't even have a Fedora build loop for 4.2, only CentOS.  Fedora will still
# work with 4.2, but you'll need to use the -d option to have the script build all SIPfoundry dependency RPMs locally.
#
         sudo sed -i -e "s/\$releasever/8/g" /etc/yum.repos.d/$(return_sipxecs_unstable_repo_name).repo
      fi
      sudo sed -i -e "s/gpgcheck=1/gpgcheck=0/g" /etc/yum.repos.d/$(return_sipxecs_unstable_repo_name).repo
   fi
}

function sudo_gem_install_and_check {
   echo -n "  Checking for (gem package) $1..."
   gem list | grep $1 > /dev/null
   if [ $? == 0 ]
   then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      sudo gem install $1
      if [ $? != 0 ]
      then
         sudo gem install $1
         if [ $? != 0 ]
         then
            sudo gem install $1 --no-rdoc
            if [ $? != 0 ]
            then
               echo "ERROR: FAILED to install - $1!" >&2
               exit 5
            fi
         fi
      fi
      echo "    DONE."
   fi
}

function return_sipxecs_installed_rpms {
   echo `rpm -qa | grep sipx | grep -v freeswitch | grep -v openfire`
}

function uninstall_sipxecs_rpms {
   SIPX_UNINSTALL=$(return_sipxecs_installed_rpms)
   echo SIPX_UNINSTALL: --$SIPX_UNINSTALL-- >> $FULL_PATH_EDE_LOGS/sipxecs_rpm_uninstall.log
   if [ -n "$SIPX_UNINSTALL" ]; then
      sudo yum -y remove $SIPX_UNINSTALL >> $FULL_PATH_EDE_LOGS/sipxecs_rpm_uninstall.log
      yum_remove_result=$?
      echo yum_remove_result: $yum_remove_result >> $FULL_PATH_EDE_LOGS/sipxecs_rpm_uninstall.log
      if [ $yum_remove_result != 0 ]; then
         echo "Expected failure result, due to XX-226 (Won't Fix.)" >> $FULL_PATH_EDE_LOGS/sipxecs_rpm_uninstall.log
      fi
   fi
   sudo rpm -e sipxproxy-cdr --noscripts &> /dev/null
   rpm -q sipxproxy-cdr &> /dev/null
   if [ $? == 0 ]; then
      echo "ERROR: Failed to uninstall sipxproxy-cdr, even with '--noscripts'!  (XX-226)" >&2
      exit 6
   fi
   if [ -n "$(return_sipxecs_installed_rpms)" ]; then
      echo "ERROR: Failed to uninstall one or more sipXecs RPMs: $(return_sipxecs_installed_rpms)" >&2
      exit 7
   fi
}

# Make a fresh EDE log dir.
sudo rm -rf $EDE_LOGS.old 2> /dev/null
mv $EDE_LOGS $EDE_LOGS.old 2> /dev/null
mkdir $EDE_LOGS

if [ -z "$SANDBOX_MODE" ]; then
   # Stop the old (which may not be running, or even installed...)
   if test -x /etc/init.d/sipxpbx
   then
      sudo /sbin/service sipxpbx stop 2> /dev/null
   elif test -x /etc/init.d/sipxecs
   then
      sudo /sbin/service sipxecs stop 2> /dev/null
   elif test -x $INSTALL/etc/init.d/sipxecs
   then
      sudo $INSTALL/etc/init.d/sipxecs stop 2> /dev/null
   fi
   sudo killall httpd 2> /dev/null

   # Uninstall any old sipXecs RPMs.
   echo "Uninstalling any old sipXecs RPMs..."
   uninstall_sipxecs_rpms

   # Dependencies that are required, but only available from SIPfoundry dependency RPMs.  (Applies to both
   # Fedora 10/11 and CentOS 5.2.)
   SIPFOUNDRY_BASE_DEPS="cppunit cppunit-devel ruby-dbi ruby-postgres sipx-openfire"

   # Dependencies that are required, but only available from SIPfoundry dependency RPMs.  (Applies to both
   # Fedora 10/11 and CentOS 5.2.)  But in the case of Fedora 10/11 these MUST be built locally.
   FREESWITCH_SIPXECS_DEPS="sipx-freeswitch sipx-freeswitch-codec-passthru-amr sipx-freeswitch-codec-passthru-g723_1 sipx-freeswitch-codec-passthru-g729 sipx-freeswitch-devel sipx-freeswitch-lang-en sipx-freeswitch-lua sipx-freeswitch-perl sipx-freeswitch-spidermonkey"

   # Uninstall any old dependency RPMs.
   DEPS_UNINSTALL="$SIPFOUNDRY_BASE_DEPS $FREESWITCH_SIPXECS_DEPS"
   if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
      # For CentOS 5.2 the BASE_DEPS also come from SIPfoundry dependency RPMs, so we'll refresh them too.
      DEPS_UNINSTALL="$DEPS_UNINSTALL $BASE_DEPS"
   fi
   echo "Uninstalling any old dependency RPMs..."
   echo DEPS_UNINSTALL: $DEPS_UNINSTALL >> $FULL_PATH_EDE_LOGS/dependency_rpm_uninstall.log
   sudo yum -y remove $DEPS_UNINSTALL >> $FULL_PATH_EDE_LOGS/dependency_rpm_uninstall.log
   yum_remove_result=$?
   echo yum_remove_result: $yum_remove_result >> $FULL_PATH_EDE_LOGS/dependency_rpm_uninstall.log
   if [ $yum_remove_result != 0 ]; then
      echo "ERROR: Dependency RPM uninstall failed, see $EDE_LOGS/dependency_rpm_uninstall.log" >&2
      exit 8
   fi
fi

# This file can be source'd by a shell (using '.' or 'source'.)
cat <<EOF > $EDE_ENV_FILE
WORKING="`pwd`"
WORKING_DIR="`pwd`"
INSTALL="`pwd`/$INSTALL"
BUILD="`pwd`/$BUILD"
CODE="`pwd`/$CODE"
LINKS="`pwd`/$LINKS"
DIST="`pwd`/$DIST"
# if you have non-git patch run this
jira-apply() {
    curl \$1 | git apply -p0 --whitespace=strip
}
# if you have git patch run this
gjira-apply() {
    curl \$1 | git am -3 --whitespace=strip
}
fisheye-links-filter() {
    sed -e 's/^r\([0-9]*\).*/http:\/\/code.sipfoundry.org\/changelog\/sipXecs\/?cs=\1/'
}
# use gsl -3 to display last 3 commits
gsl() {
    git svn log \$1 --reverse | fisheye-links-filter
}
parse_git_branch() {
  git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e 's/* \(.*\)/\1/'
}
EOF
# Note: More content is added below in the Eclipse readiness section.

# Start removing the old and creating the new.
sudo rm -rf $INSTALL $BUILD $RPMBUILD $LINKS $DIST $DEP_RPM_TOPDIR $SIPX_RPM_TOPDIR $ECLIPSE_WORKSPACE $EDE_BIN
mv env env.DELETE_ME 2> /dev/null
mv eclipse-windowprefs.txt eclipse-windowprefs.txt.DELETE_ME 2> /dev/null
for lib_comp in $(ls $CODE/lib);
do
   sudo rm -rf $CODE/lib/$lib_comp/build 2> /dev/null
done
mkdir $INSTALL

if [ -z "$SANDBOX_MODE" ]; then
   sudo rm -rf /etc/init.d/sipxecs /etc/init.d/sipxpbx
   sudo rm -rf /etc/yum.repos.d/sipxecs-dependencies-local.repo
   sudo rm -rf /etc/yum.repos.d/$(return_sipxecs_unstable_repo_name).repo
fi

# Record useful info.
echo BUILD_ALL_DEPENDENCIES - $BUILD_ALL_DEPENDENCIES >> $FULL_PATH_EDE_LOGS/info.log
echo BUILD_RPMS - $BUILD_RPMS >> $FULL_PATH_EDE_LOGS/info.log
echo CODE - $CODE >> $FULL_PATH_EDE_LOGS/info.log
echo Distribution - $(return_uname_distro_id) >> $FULL_PATH_EDE_LOGS/info.log

if [ -z "$SANDBOX_MODE" ]; then
   # Build any dependency RPMs which may be specified and/or required.
   if [ $BUILD_ALL_DEPENDENCIES ]; then
      # Build all those required by the distribution.
      if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
         DISTRO=centos5
      elif [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 -o $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
         DISTRO=f10
      fi
      # Manually add sipx-openfire, since it's listed in the CUSTOM_PACKAGES of the lib/Makefile.
      DEPENDENCY_TARGET="$DISTRO sipx-openfire"
      BUILD_DEPENDENCY_TARGET="yup"
   else
      if [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 -o $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
         # The Fedora 8 sipxecs-unstable FreeSWITCH RPMs will fail to install
         # on Fedora 10/11, so we need to build them locally.
         DEPENDENCY_TARGET=freeswitch
         BUILD_DEPENDENCY_TARGET="yup"
      fi
   fi
   if [ $BUILD_DEPENDENCY_TARGET ]; then
      echo "Building dependency RPMs covered under target(s): $DEPENDENCY_TARGET..."
      mv ~/.rpmmacros ~/.rpmmacros.old 2> /dev/null
      echo "%_topdir      $FULL_PATH_DEP_RPM_TOPDIR" > ~/.rpmmacros
      rpmdev-setuptree
      pushd $CODE/lib > /dev/null
      rm -rf cache-file
      make $DEPENDENCY_TARGET DESTDIR=$FULL_PATH_DIST LIBSRC=$FULL_PATH_DIST/libsrc &> $FULL_PATH_EDE_LOGS/dependency_rpm_build.log
      dep_make_result=$?
      echo DEPENDENCY_TARGET: $DEPENDENCY_TARGET >> $FULL_PATH_EDE_LOGS/dependency_rpm_build.log
      echo $FULL_PATH_DIST/RPM contents: >> $FULL_PATH_EDE_LOGS/dependency_rpm_build.log
      ls -la $FULL_PATH_DIST/RPM >> $FULL_PATH_EDE_LOGS/dependency_rpm_build.log &> /dev/null
      echo dep_make_result: $dep_make_result >> $FULL_PATH_EDE_LOGS/dependency_rpm_build.log
      if [ $dep_make_result != 0 ]; then
         echo "ERROR: Dependency RPM build failed, see $EDE_LOGS/dependency_rpm_build.log" >&2
         exit 9
      fi

      echo "Creating local repository..."
      echo [sipxecs-dependencies-local] > $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo
      echo name=sipXecs dependencies local >> $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo
      echo baseurl=file://$FULL_PATH_DIST/RPM >> $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo
      echo enabled=1 >> $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo
      echo gpgcheck=0 >> $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo
      sudo cp $FULL_PATH_EDE_LOGS/sipxecs-dependencies-local.repo /etc/yum.repos.d/
      sudo createrepo $FULL_PATH_DIST/RPM > /dev/null 2> $FULL_PATH_EDE_LOGS/createrepo-error.log
      popd > /dev/null
   fi

   # Get ready to install the SIPfoundry dependency RPMs with one big yum command below, regardless
   # of whether it's from the local or sipxecs-unstable repo.
   sudo yum -y update yum
   BIG_DEP_INSTALL="$SIPFOUNDRY_BASE_DEPS"

   # CentOS 5.2?
   if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
      # Unlike Fedora 10/11, these aren't available from the standard repository under CentOS 5.2.
      BIG_DEP_INSTALL="$BIG_DEP_INSTALL $BASE_DEPS"
   fi

   # Were all/any SIPfoundry dependency RPMs built locally?
   if [ $BUILD_ALL_DEPENDENCIES ]; then
      # All dependencies were built locally, and must be installed locally (from sipxecs-dependencies-local,
      # which was created above.)
      BIG_DEP_INSTALL="$BIG_DEP_INSTALL $FREESWITCH_SIPXECS_DEPS"
   else
      # At least some dependencies must be installed from the sipxecs-unstable repo.
      add_sipxecs_unstable_repo

      # Is this Fedora 10/11?
      if [ $(return_uname_distro_id) == $DISTRO_ID_Fedora10 -o $(return_uname_distro_id) == $DISTRO_ID_Fedora11 ]; then
         # Fedora 10/11 makes things tricky here.  It mostly uses the Fedora 8 RPMs on sipxecs-unstable,
         # except that the FreeSWITCH ones will fail to install.  So they were built locally and must be installed
         # locally (from sipxecs-dependencies-local, which was created above.)
         echo FREESWITCH_SIPXECS_DEPS: $FREESWITCH_SIPXECS_DEPS >> $FULL_PATH_EDE_LOGS/dependency_rpm_install.log
         sudo yum -y install --disablerepo=sipxecs-unstable $FREESWITCH_SIPXECS_DEPS >> $FULL_PATH_EDE_LOGS/dependency_rpm_install.log 2> $FULL_PATH_EDE_LOGS/FREESWITCH_SIPXECS_DEPS-error.log
      else
         # The FreeSWITCH dependencies can also be installed from the sipxecs-unstable repo.
         BIG_DEP_INSTALL="$BIG_DEP_INSTALL $FREESWITCH_SIPXECS_DEPS"
      fi
   fi
   echo BIG_DEP_INSTALL: $BIG_DEP_INSTALL >> $FULL_PATH_EDE_LOGS/dependency_rpm_install.log
   sudo yum -y install $BIG_DEP_INSTALL >> $FULL_PATH_EDE_LOGS/dependency_rpm_install.log
   if [ $? != 0 ]; then
      echo "ERROR: Dependency RPM install failed, see $EDE_LOGS/dependency_rpm_install.log" >&2
      exit 10
   fi

   # The rubygems package may have been installed by the base script (Fedora 10/11 - standard repo) or
   # by this script (CentOS 5.2 - SIPfoundry dependency RPMs.)  For simplicity though, we always
   # attempt to update and install gems here in this script.
   sudo gem update --system
   GEM_PACKAGES="file-tail rake"
   for package in $GEM_PACKAGES;
   do
      sudo_gem_install_and_check $package
   done
fi

# Record the java version being used.
sudo /usr/sbin/alternatives --display java >> $FULL_PATH_EDE_LOGS/java_alternatives.log
sudo /usr/sbin/alternatives --display javac >> $FULL_PATH_EDE_LOGS/java_alternatives.log

# Some handy scripts.
mkdir $EDE_BIN
cat <<EOF > $FULL_PATH_EDE_BIN/full_conf_build
#!/bin/bash
#
# Generated by '$0' on "`date`"
START_DATE=\`date\`

if [ "\`whoami\`" == root ]; then
  echo "" >&2
  echo "ERROR: Do NOT run this script as root (or with sudo.)" >&2
  echo "" >&2
  exit 1
fi

EOF
chmod +x $FULL_PATH_EDE_BIN/full_conf_build
cp $FULL_PATH_EDE_BIN/full_conf_build $FULL_PATH_EDE_BIN/rebuild

echo "Running autoreconf..."
pushd $CODE > /dev/null
FULL_CODE_PATH=`pwd`
AUTORECONF_COMMAND="autoreconf -ifv"
cat <<EOF >> $FULL_PATH_EDE_BIN/full_conf_build
pushd $FULL_CODE_PATH
$AUTORECONF_COMMAND
if [ \$? != 0 ]; then
   echo \"ERROR: autoreconf failed!\" >&2
   exit 1
fi
popd

EOF
${AUTORECONF_COMMAND} &> $FULL_PATH_EDE_LOGS/autoreconf_output.log
if [ $? != 0 ]
then
   echo "ERROR: sipXecs autoreconf failed, see $EDE_LOGS/autoreconf_output.log" >&2
   exit 11
fi
popd > /dev/null

# How are we building sipXecs?
CONFIGURE_FLAGS="--enable-reports --enable-agent --enable-cdr --enable-mrtg"
if [ $BUILD_RPMS ]; then
   # RPM build/install  - http://wiki.sipfoundry.org/display/oldxx/Building+RPMs
   mv ~/.rpmmacros ~/.rpmmacros.old 2> /dev/null
   echo "%_topdir      $FULL_PATH_SIPX_RPM_TOPDIR" > ~/.rpmmacros
   rpmdev-setuptree
   unset TMPDIR
   BUILDER=EDE-$(return_uname_distro_id)
   FULL_INSTALL_PATH=/usr
   ETC_AND_VAR_PATH=""
   echo "Running RPM configure..."
   ACTUAL_BUILD_DIR=$WORKING_DIR/$RPMBUILD
   mkdir -p $ACTUAL_BUILD_DIR
   pushd $ACTUAL_BUILD_DIR > /dev/null
   CONFIGURE_COMMAND="$FULL_CODE_PATH/configure --srcdir=$FULL_CODE_PATH --cache-file=`pwd`/ac-cache-file SIPXPBXUSER=`whoami` --with-distdir=$FULL_PATH_DIST $CONFIGURE_FLAGS"
   ${CONFIGURE_COMMAND} &> $FULL_PATH_EDE_LOGS/rpm_configure_output.log
   config_result=$?
   cp config.log $FULL_PATH_EDE_LOGS
   if [ $config_result != 0 ]; then
      echo "ERROR: sipXecs RPM configure failed, see $EDE_LOGS/rpm_configure_output.log" >&2
      exit 12
   fi

   echo "Running RPM make..."
   MAKE_COMMAND="make rpm"
   ${MAKE_COMMAND} &> $FULL_PATH_EDE_LOGS/rpm_make_output.log
   if [ $? != 0 ]; then
      echo "ERROR: sipXecs RPM make failed, see $EDE_LOGS/rpm_make_output.log" >&2
      exit 13
   fi
   echo "Completed make..."
   popd > /dev/null

   # Re-install the RPMs properly.
   echo "Uninstalling the sipXecs RPMs that were improperly installed as they were built."
   uninstall_sipxecs_rpms
   if [ -z "$SKIP_LOCAL_SETUP_RUN" ]; then
      echo "Re-installing the sipXecs RPMs properly..."
      sudo yum -y localinstall $DIST/RPM/sipx*.rpm --nogpgcheck &> $FULL_PATH_EDE_LOGS/rpm_install_output.log
      if [ $? != 0 ]; then
         echo "ERROR: sipXecs RPM install failed, see $EDE_LOGS/rpm_install_output.log" >&2
         exit 14
      fi
   fi
else
   # Designer Makefile build/install
   echo "Running \"designer\" configure..."
   pushd $INSTALL > /dev/null
   FULL_INSTALL_PATH=`pwd`
   ETC_AND_VAR_PATH=$FULL_INSTALL_PATH
   popd > /dev/null
   ACTUAL_BUILD_DIR=$WORKING_DIR/$BUILD
   mkdir -p $ACTUAL_BUILD_DIR
   pushd $ACTUAL_BUILD_DIR > /dev/null
   CONFIGURE_COMMAND="$FULL_CODE_PATH/configure --srcdir=$FULL_CODE_PATH --cache-file=`pwd`/ac-cache-file SIPXPBXUSER=`whoami` JAVAC_OPTIMIZED=off JAVAC_DEBUG=on SIPX_BUILD_LABEL=$WORKING_DIR --prefix=$FULL_INSTALL_PATH $CONFIGURE_FLAGS"
   ${CONFIGURE_COMMAND} &> $FULL_PATH_EDE_LOGS/designer_configure_output.log
   config_result=$?
   cp config.log $FULL_PATH_EDE_LOGS
   if [ $config_result != 0 ]; then
      echo "ERROR: sipXecs \"designer\" configure failed, see $EDE_LOGS/designer_configure_output.log" >&2
      exit 15
   fi

   echo "Running \"designer\" make..."
   MAKE_COMMAND="make recurse TARGETS=\"all install\""
   echo $MAKE_COMMAND > ./tmp
   . ./tmp &> $FULL_PATH_EDE_LOGS/designer_make_output.log
   make_result=$?
   rm -rf ./tmp
   if [ $make_result != 0 ]; then
      echo "ERROR: sipXecs \"designer\" make failed, see $EDE_LOGS/designer_make_output.log" >&2
      exit 16
   fi
   echo "Completed make..."
   popd > /dev/null

   if [ -z "$SANDBOX_MODE" ]; then
      # Install the gems that were just built (and hidden away...)
      sudo gem install $FULL_INSTALL_PATH/lib/ruby/gems/1.8/cache/*.gem &> $FULL_PATH_EDE_LOGS/sipxecs_gem_install.log
      if [ $? != 0 ]; then
         echo "ERROR: sipXecs gem install failed, see $EDE_LOGS/sipxecs_gem_install.log" >&2
         exit 17
      fi

      # This is needed so often, we might as well make it easily available with "sudo /sbin/service sipxecs xxx",
      # and started automatically after reboot.
      sudo rm -rf /etc/init.d/sipxecs
      sudo ln -s $FULL_INSTALL_PATH/etc/init.d/sipxecs /etc/init.d/sipxecs

      # Cause the logs to be rotated.
      sudo rm -rf /etc/logrotate.d/sipxchange
      sudo ln -s $FULL_INSTALL_PATH/etc/logrotate.d/sipxchange /etc/logrotate.d/sipxchange

      # Adjust the TFTP/FTP directory.
      TFTP_PATH=$FULL_INSTALL_PATH/var/sipxdata/configserver/phone/profile/tftproot
      ruby -e 'path=""; ARGV[0].split("/").each {|x| path+=x+"/"; `sudo chmod g+x #{path}`}' $TFTP_PATH
      sudo rm -rf /tftpboot
      sudo ln -s $TFTP_PATH /tftpboot

      # Undo damage that might have been done by a previous RPM build.
      sudo sed -i -e 's/-s \/var\/sipxdata\/configserver\/phone\/profile\/tftproot/-s \/tftpboot/g' /etc/xinetd.d/tftp
      sudo /sbin/service xinetd restart

      # Enable the FTP users, also using the /tftpboot directory.
      add_ftp_user PlcmSpIp PlcmSpIp
      add_ftp_user lvp2890 28904all
   fi
fi

# Finish the scripts.
cat <<EOF >> $FULL_PATH_EDE_BIN/full_conf_build
pushd $ACTUAL_BUILD_DIR
$CONFIGURE_COMMAND
if [ \$? != 0 ]; then
   echo \"ERROR: configure failed!\" >&2
   exit 1
fi
popd
EOF
cat <<EOF >> $FULL_PATH_EDE_BIN/tmp
pushd $ACTUAL_BUILD_DIR
$MAKE_COMMAND
if [ \$? != 0 ]; then
   echo \"ERROR: make failed!\" >&2
   exit 1
fi
popd
echo ""
echo "Start   : \$START_DATE"
echo -n "Complete: "
date
echo ""
echo "DONE!"
echo ""
EOF
cat $FULL_PATH_EDE_BIN/tmp >> $FULL_PATH_EDE_BIN/full_conf_build
cat $FULL_PATH_EDE_BIN/tmp >> $FULL_PATH_EDE_BIN/rebuild
rm -rf $FULL_PATH_EDE_BIN/tmp

# A script for launching Eclipse with the appropriate parameters, 
cat <<EOF >> $FULL_PATH_EDE_BIN/ede-eclipse
#!/bin/bash
#
# Generated by '$0' on "`date`"

# Launch...
eclipse -data $FULL_PATH_ECLIPSE_WORKSPACE -vmargs -Xmx1024M -XX:PermSize=1024M -Dorg.eclipse.swt.internal.gtk.disablePrinting -Djava.library.path=/usr/lib

# Remove these directories, since they cause test-integration to fail with mysterious errors.
find $FULL_PATH_CODE/sipXconfig -type d -name bin.eclipse | xargs rm -rf 

EOF
chmod +x $FULL_PATH_EDE_BIN/ede-eclipse

# Another script useful for sipXconfig development.
cat <<EOF >> $FULL_PATH_EDE_BIN/config_rebuild_plus
#!/bin/bash
#
# Generated by '$0' on "`date`"
START_DATE=\`date\`

if [ "\`whoami\`" == root ]; then
  echo "" >&2
  echo "ERROR: Do NOT run this script as root (or with sudo.)" >&2
  echo "" >&2
  echo -e '\a'
  exit 1
fi

# Rebuild
pushd $FULL_PATH_BUILD/sipXconfig
make
if [ \$? != 0 ]; then
   echo \"ERROR: sipXconfig make failed!\" >&2
   echo -e '\a'
   exit 2
fi
make install
if [ \$? != 0 ]; then
   echo \"ERROR: sipXconfig make install failed!\" >&2
   echo -e '\a'
   exit 4
fi
popd

# Restart
$FULL_PATH_INSTALL/bin/sipxproc --restart ConfigServer

# Remove these directories, since they cause test-integration to fail with mysterious errors.
find $FULL_PATH_CODE/sipXconfig -type d -name bin.eclipse | xargs rm -rf 

# Full unit tests
pushd $FULL_PATH_CODE/sipXconfig
ant precommit
if [ \$? != 0 ]; then
   echo \"ERROR: sipXconfig ant precommit failed!\" >&2
   echo -e '\a'
   exit 5
fi
popd

echo ""
echo "Start   :" \$START_DATE
echo -n "Complete: "
date
echo ""
echo "`basename \\$0` DONE!"
echo ""
echo -e '\a'

EOF
chmod +x $FULL_PATH_EDE_BIN/config_rebuild_plus

# Create some helpful links.
mkdir $LINKS
pushd $LINKS > /dev/null
ln -s $ETC_AND_VAR_PATH/var/log/sipxpbx log
ln -s $ETC_AND_VAR_PATH/var/sipxdata/configserver/phone/profile/tftproot tftproot
ln -s $ETC_AND_VAR_PATH/var/sipxdata/sipdb sipdb
ln -s $ETC_AND_VAR_PATH/etc/sipxpbx home
ln -s $FULL_INSTALL_PATH/bin bin
ln -s $FULL_INSTALL_PATH/share/sipxecs/process.d process.d
popd > /dev/null

if [ -z "$SANDBOX_MODE" ]; then
   # Easy scripts to start, stop, restart, and get status.
   echo sudo $ETC_AND_VAR_PATH/etc/init.d/sipxecs start > /tmp/sstart
   sudo mv /tmp/sstart /usr/bin/
   sudo chmod a+rx /usr/bin/sstart
   echo sudo $ETC_AND_VAR_PATH/etc/init.d/sipxecs stop > /tmp/sstop
   sudo mv /tmp/sstop /usr/bin/
   sudo chmod a+rx /usr/bin/sstop
   echo sudo $ETC_AND_VAR_PATH/etc/init.d/sipxecs status > /tmp/sstatus
   sudo mv /tmp/sstatus /usr/bin/
   sudo chmod a+rx /usr/bin/sstatus
   echo sudo $ETC_AND_VAR_PATH/etc/init.d/sipxecs stop > /tmp/srestart
   echo sudo $ETC_AND_VAR_PATH/etc/init.d/sipxecs start >> /tmp/srestart
   sudo mv /tmp/srestart /usr/bin/
   sudo chmod a+rx /usr/bin/srestart

   # Clear any database contents that might be left over from the last install.
   $FULL_INSTALL_PATH/bin/sipxconfig.sh --database drop create &> $FULL_PATH_EDE_LOGS/sipxconfig_drop_create.log
   $FULL_INSTALL_PATH/bin/sipxconfig.sh --first-run &> $FULL_PATH_EDE_LOGS/sipxconfig_first-run.log
fi

# Fix FreeSWITCH
sudo $FULL_INSTALL_PATH/bin/freeswitch.sh --configtest &> $FULL_PATH_EDE_LOGS/freeswitch_configtest.log

# Eclipse readiness.
mkdir -p $FULL_PATH_ECLIPSE_WORKSPACE
echo alias \'ede-eclipse=$FULL_PATH_EDE_BIN/ede-eclipse \&\' >> $EDE_ENV_FILE
echo SIPX_MYBUILD=\"`pwd`/$BUILD\" >> $EDE_ENV_FILE
echo SIPX_MYBUILD_OUT=\"`pwd`/$BUILD\" >> $EDE_ENV_FILE
FULL_PATH_ECLIPSE_SETTINGS=$FULL_PATH_ECLIPSE_WORKSPACE/.metadata/.plugins/org.eclipse.core.runtime/.settings
mkdir -p $FULL_PATH_ECLIPSE_SETTINGS
cat <<EOF > $FULL_PATH_ECLIPSE_SETTINGS/org.eclipse.core.resources.prefs
# Generated by EDE
version=1
eclipse.preferences.version=1
pathvariable.SIPX_OUT=$WORKING_DIR/$INSTALL/bin
EOF
cat <<EOF > $FULL_PATH_ECLIPSE_SETTINGS/org.eclipse.jdt.core.prefs
# Generated by EDE
eclipse.preferences.version=1
org.eclipse.jdt.core.classpathVariable.SIPX_SRC_TOP=$WORKING_DIR/$CODE
org.eclipse.jdt.core.classpathVariable.SIPX_BUILD_TOP=$WORKING_DIR/$BUILD
org.eclipse.jdt.core.classpathVariable.JAVA_LIBDIR=/usr/share/java
org.eclipse.jdt.core.classpathVariable.OPENFIRE_ROOT=/opt/openfire
org.eclipse.jdt.core.classpathVariable.SIPX_BUILD=$WORKING_DIR/$BUILD/sipXconfig
org.eclipse.jdt.core.classpathVariable.SIPX_COMMONS=$WORKING_DIR/$CODE/sipXcommons
org.eclipse.jdt.core.classpathVariable.SIPX_CONFIG=$WORKING_DIR/$CODE/sipXconfig
org.eclipse.jdt.core.classpathVariable.SIPX_PREFIX=$WORKING_DIR/$INSTALL
EOF
if [ -n "$BUILD_RPMS" ]; then
   echo "Skipping sipXconfig Eclipse setup..."
else
   echo "Running sipXconfig-specific Eclipse setup..."
   pushd $CODE/sipXconfig > /dev/null
   echo "   ant default..."
   ant default >>$FULL_PATH_EDE_LOGS/sipxconfig_eclipse_setup.log 2>&1
   if [ $? != 0 ]; then
      echo "ERROR: sipXconfig Eclipse 'ant default' failed.  See $EDE_LOGS/sipxconfig_eclipse_setup.log" >&2
      exit 18
   fi
   popd > /dev/null
   echo 'SIPXCONFIG_OPTS="-Xdebug -Xnoagent -Djava.compiler=NONE -Xrunjdwp:transport=dt_socket,server=y,suspend=n,address=4241"' \
      > $ETC_AND_VAR_PATH/etc/sipxpbx/sipxconfigrc
fi

# Non-interactive section complete.
echo ""
echo "Start   : $START_DATE"
echo -n "Complete: "
date
echo ""

# Local setup and run?
if [ -n "$SKIP_LOCAL_SETUP_RUN" ]; then
   echo "Skipping sipxecs-setup..."
else
   $FULL_INSTALL_PATH/bin/sipxecs-setup

   # Display the current service status, for reference.
   $ETC_AND_VAR_PATH/etc/init.d/sipxecs status
fi

echo ""
echo "DONE!"
echo ""
echo "(Don't forget to run 'source $EDE_ENV_FILE'!)"
echo ""


