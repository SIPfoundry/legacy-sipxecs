#!/bin/bash
#
# Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# ede_base_root.sh
#
# See http://wiki.sipfoundry.org/display/xecsdev/Express+Development+Environment+Setup for insptructions.

START_DATE=`date`

if [ "`whoami`" != root ]; then
  echo "You must be root in order to run this script." >&2
  exit 1
fi

DEVEL_USER=sipxchange
DEFAULT_PASSWORD=PingMe

if [ $# -gt 1 ]; then
  echo "Usage: ${0} [Username]" >&2
  exit 2 
fi
if [ $# -gt 0 ]; then
  DEVEL_USER=$1
fi

echo ""
echo "Using development username: $DEVEL_USER."
echo "" 

# See http://wiki.sipfoundry.org/display/xecsdev/Express+Development+Environment+Setup#ExpressDevelopmentEnvironmentSetup-f.ConfirmtheFQDN%28FullyQualifiedDomainName%29
HOSTNAME_F=`hostname -f`
host $HOSTNAME_F
if [ $? != 0 ]; then
  echo "ERROR: The domain name returned by 'hostname -f' does not resolve to an IP address." >&2
  exit 3
fi
echo $HOSTNAME_F | grep -e "\." &> /dev/null
if [ $? != 0 ]; then
  echo "ERROR: The domain name returned by 'hostname -f' is not fully qualified." >&2
  exit 4
fi

DISTRO_EXIT_ERROR=5 # exit
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
      exit $DISTRO_EXIT_ERROR >&2 
   else
      echo -n "Unsupported Linux distribution: "
      uname -a | cut -d" " -f3
      exit $DISTRO_EXIT_ERROR >&2
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

echo Start: `date` > ede_base_root.log
echo Development User: $DEVEL_USER >> ede_base_root.log
echo "\$#:" $# >> ede_base_root.log
echo "\$1:" $1 >> ede_base_root.log

# Get ready for some big yum work.
yum -y update yum
yum clean all

# Used in this script.
yum -y install ruby yum-utils

function wget_retry {
   P_OPT=""
   if [ $# == 2 ]; then
      P_OPT="-P $2 "
   fi
   echo "  wget_retry for $P_OPT$1..." 
   wget $P_OPT$1
   if [ $? != 0 ]; then
      wget $P_OPT$1
      if [ $? != 0 ]; then
         wget $P_OPT$1
         if [ $? != 0 ]; then
            echo "    FAILED!" >&2
            exit 6
         fi               
      fi      
   fi
   echo "    SUCCESS." 
}

function wget_repofile {
   filename=`echo $1 | ruby -e 'puts File.basename(STDIN.readline)'`
   if [ ! -f /etc/yum.repos.d/$filename ]; then
      wget_retry $1 /etc/yum.repos.d
   fi
}

function rpm_file_install_and_check {
   FILENAME=`ruby -e 'puts File.basename(ARGV[0])' $1`
   BASENAME=`ruby -e 'puts ARGV[0].split(/-[0-9]/)[0]' $FILENAME`

   echo -n "  Checking for (rpm package) $BASENAME..." 
   rpm -q $BASENAME > /dev/null
   if [ $? == 0 ]; then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      rm -rf {$FILENAME}*
      wget_retry $1
      rpm -i $FILENAME
      if [ $? != 0 ]; then
         echo "    FAILED to install - $BASENAME!" >&2 
         exit 7
      fi
      rpm -q $BASENAME > /dev/null
      if [ $? = 1 ]; then
         echo "    no error, but then FAILED to find installed - $BASENAME!" >&2
         exit 8
      fi
   fi
}

function yum_install_and_check {
   echo -n "  Checking for (yum package) $1..." 
   rpm -q $1 > /dev/null
   if [ $? == 0 ]; then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      yum -y install $1 
      rpm -q $1 > /dev/null
      if [ $? != 0 ]; then
         yum -y install $1 
         rpm -q $1 > /dev/null
         if [ $? != 0 ]; then
            yum -y install $1 
            rpm -q $1 > /dev/null
            if [ $? != 0 ]; then
               echo "    FAILED to install - $1!" >&2 
               exit 9
            fi
         fi         
      fi
      echo "    DONE."
   fi
}

function SWITCH_install_latest_version_rpm {
   echo Installing $1 from switch.ch...
   rm -rf index.html
   wget_retry ftp://mirror.switch.ch/mirror/epel/5/i386/
   grep -e i386/$1-[0-9] index.html > /dev/null
   if [ $? != 0 ]; then
      echo "ERROR: $1 not found at switch.ch." >&2
      exit 10
   fi
   url=`grep -e i386/$1-[0-9] index.html | cut -d"\"" -f2 | sort | tail -1`
   rpm_file_install_and_check $url
}

function centos_manual_install_packages {
   DIR_NAME=centos_manual_install_packages

   rm -rf $DIR_NAME
   mkdir -p $DIR_NAME
   pushd $DIR_NAME

   # git - yikes!  (This sure is a lot easier on Fedora....)
   git --version &> /dev/null
   if [ $? == 0 ]; then
      echo git is already installed...
   else
      echo Installing git...
      rm -rf git
      mkdir -p git
      pushd git
      rm -rf download
      wget_retry http://git-scm.com/download
      stable_version=`grep "<div id=\"ver\">" download | cut -f4 -d"v" | cut -f1 -d"<"`
      filename=git-$stable_version.tar.gz
      wget_retry http://kernel.org/pub/software/scm/git/$filename
      tar xzf $filename
      cd git-$stable_version
      make prefix=/usr install
      if [ $? != 0 ]; then
         echo "ERROR: Failed to install git." >&2
         exit 11
      fi
      popd
      yum -y install subversion-perl
      if [ $? != 0 ]; then
         echo "ERROR: Failed to yum install subversion-perl." >&2
         exit 12
      fi
      (echo "no") |  perl -MCPAN -e 'install "Term::ReadKey"'
      if [ $? != 0 ]; then
         echo "ERROR: Failed to perl CPAN install Term::ReadKey" >&2
         exit 13 
      fi
   fi

   # Thank you SWITCH!
   rm -rf SWITCH
   mkdir -p SWITCH
   pushd SWITCH
   rm -rf index.html
   SWITCH_PACKAGES="scons lzo lzo-devel fakeroot-libs fakeroot rpmdevtools"
   for package in $SWITCH_PACKAGES;
   do
      SWITCH_install_latest_version_rpm $package
   done
   popd

   # manual_install_centos_packages
   popd
}

# Disable SELinux (could be done from a GUI install, but just to be sure....)
if [ ! -e /etc/selinux/config_ORIG ] 
then
  cp /etc/selinux/config /etc/selinux/config_ORIG
fi
echo SELINUX=disabled > /etc/selinux/config # This prevents it from starting following reboots.
echo 0 >/selinux/enforce # This stops it right now, without a reboot.

# Disable the Firewall (could be done from a GUI install, but just to be sure....)
/sbin/service iptables stop
/sbin/chkconfig iptables off

# This tool binds to port 5060, which will obviously cause problems.
killall sip-redirect
yum -y remove sip-redirect &> /dev/null

# Install the required packages.
YUM_PACKAGES="gcc gcc-c++ autoconf automake libtool subversion subversion-perl rpm-build httpd httpd-devel openssl-devel jpackage-utils pcre-devel expat-devel unixODBC-devel junit ant-commons-logging postgresql-server zlib-devel postgresql-devel postgresql-odbc alsa-lib-devel gnutls-devel mysql-devel ncurses-devel python-devel ruby ruby-devel ruby-rdoc bind tftp-server doxygen zip which unzip createrepo ant-junit mod_ssl libXp libpng-devel libart_lgpl-devel freetype freetype-devel gdb gdbm-devel mysql-devel ncurses-devel vsftpd mod_perl-devel dhcp net-snmp-utils net-snmp-devel net-snmp-perl ntp yum-utils java-1.6.0-openjdk java-1.6.0-openjdk-devel redhat-rpm-config ant ant-trax ant-nodeps jakarta-commons-collections jakarta-commons-beanutils log4j mrtg stunnel logrotate"

# Differences in certain packages between distros.  (See also function centos_manual_install_packages.)
if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
   # CentOS 5.2

   # This is only available from jpackage, yet other RPMs there cause conflicts.
   rpm -q jakarta-commons-net &> /dev/null
   if [ $? != 0 ]; then
      wget_repofile http://www.jpackage.org/jpackage.repo
      DIR_NAME=centos_jakarta-commons-net_problems
      rm -rf $DIR_NAME
      mkdir -p $DIR_NAME
      pushd $DIR_NAME
      yumdownloader jakarta-commons-net
      if [ $? != 0 ]; then
         echo "ERROR: Failed to download jakarta-commons-net on CentOS 5." >&2
         exit 14
      fi
      rpm -ihv jakarta-commons-net*.rpm --nodeps
      if [ $? != 0 ]; then
         echo "ERROR: Failed to install jakarta-commons-net on CentOS 5." >&2
         exit 15
      fi
      popd
      rm -rf /etc/yum.repos.d/jpackage.repo
   fi

   YUM_PACKAGES="$YUM_PACKAGES curl-devel"
      # Note: $BASE_DEPS are NOT available from the standard CentOS 5.2 repos.  They will need to be installed
      # from SIPfoundry dependency RPMs by the next script.  (They will either be built locally, or installed 
      # from the sipxecs-unstable repo.)
else
   # Fedora 10/11

   # $BASE_DEPS are available from the standard Fedora 10/11 repos, so install them now...
   YUM_PACKAGES="$YUM_PACKAGES $BASE_DEPS libcurl-devel rpmdevtools git git-svn lzo-devel scons perl-ExtUtils-Embed jakarta-commons-net"
fi
for package in $YUM_PACKAGES; do
   yum_install_and_check $package
done

# Many CentOS packages have no repo available, so must be either installed from RPM
# manually, or built and installed from scratch.
if [ $(return_uname_distro_id) == $DISTRO_ID_CentOS5 ]; then
   centos_manual_install_packages
fi

# Ensure we're using the correct version of java.
/usr/sbin/alternatives --set java /usr/lib/jvm/jre-1.6.0-openjdk/bin/java
/usr/sbin/alternatives --display java > $FULL_PATH_EDE_LOGS/java_alternatives.log
/usr/sbin/alternatives --set javac /usr/lib/jvm/java-1.6.0-openjdk/bin/javac

# See if the development user already exists.
id $DEVEL_USER 2> /dev/null
if [ $? != 0 ]
then
   # No, so create it and change the password.
   /usr/sbin/useradd $DEVEL_USER
   echo $DEFAULT_PASSWORD | passwd $DEVEL_USER --stdin
fi

# Give the wheel group password-less sudo privileges.
sed -i -e "s/# %wheel[\t]ALL=(ALL)[\t]NOPASSWD/%wheel\tALL=(ALL)\tNOPASSWD/g" /etc/sudoers 

# Add the development user to the wheel group.
ETC_GROUP_FILE="/etc/group"
WHEEL_GROUP_ORIG=`grep wheel $ETC_GROUP_FILE`
TMP=`echo $WHEEL_GROUP_ORIG | grep $DEVEL_USER | cut -d: -f4`
MISSING=`ruby -e 'ARGV[0].split(",").each {|x| if x == ARGV[1] then exit end}; puts "missing"' $TMP $DEVEL_USER`
if [ $MISSING ]
then
   sed -i -e "s/$WHEEL_GROUP_ORIG/$WHEEL_GROUP_ORIG,$DEVEL_USER/g" $ETC_GROUP_FILE
fi

# Enable TFTP.  Make sure it's using /tftpboot, which will be later be replaced with a symbolic 
# link by the $DEVEL_USER.  (The Fedora 10/11 default is /var/lib/tftpboot.)
sed -i -e "s/[\t]disable[\t][\t][\t]= yes/\tdisable\t\t\t= no/g" /etc/xinetd.d/tftp
sed -i -e "s/\/var\/lib\/tftpboot/\/tftpboot/g" /etc/xinetd.d/tftp
/sbin/service xinetd stop > /dev/null
/sbin/service xinetd start

# Enable FTP.
/sbin/chkconfig vsftpd on
/sbin/service vsftpd stop > /dev/null
/sbin/service vsftpd start

# Enable postgresql.
/sbin/service postgresql initdb > /dev/null
/sbin/chkconfig postgresql on
/sbin/service postgresql stop > /dev/null
/sbin/service postgresql start

echo Complete: `date` >> ede_base_root.log

echo ""
echo "Start   : $START_DATE"
echo -n "Complete: "
date
echo ""
echo "Proceed to EDE Step 3."
echo ""
