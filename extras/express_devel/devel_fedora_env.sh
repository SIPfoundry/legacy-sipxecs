#!/bin/bash
#
# Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# See http://sipx-wiki.calivia.com/index.php/Express_Development_Environment_Setup for instructions.

DEVEL_USER=sipx
DEFAULT_PASSWORD=PingMe

if [ $# == 1 ]; then
   DEVEL_USER=$1
fi

function wget_retry {
   P_OPT=""
   if [ $# == 2 ]
   then
      P_OPT="-P $2 "
   fi
   echo "  wget_retry for $P_OPT$1..." 
   wget $P_OPT$1
   if [ $? != 0 ]
   then
      wget $P_OPT$1
      if [ $? != 0 ]
      then
         wget $P_OPT$1
         if [ $? != 0 ]
         then
            wget $P_OPT$1
            if [ $? != 0 ]
            then
               echo "    FAILED!" 
               exit 2
            fi               
         fi               
      fi      
   fi
   echo "    SUCCESS." 
}

function rpm_file_install_and_check {
   FILENAME=`ruby -e 'puts File.basename(ARGV[0])' $1`
   BASENAME=`ruby -e 'puts ARGV[0].split(/-[0-9]/)[0]' $FILENAME`

   echo -n "  Checking for (rpm package) $BASENAME..." 
   rpm -q $BASENAME > /dev/null
   if [ $? = 0 ]
   then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      rm -rf {$FILENAME}*
      wget_retry $1
      rpm -i $FILENAME
      if [ $? != 0 ]
      then
         echo "    FAILED to install - $BASENAME!" 
         exit 3
      fi
      rpm -q $BASENAME > /dev/null
      if [ $? = 1 ]
      then
         echo "    no error, but then FAILED to find installed - $BASENAME!" 
         exit 3
      fi
   fi
}

function yum_install_and_check {
   echo -n "  Checking for (yum package) $1..." 
   rpm -q $1 > /dev/null
   if [ $? = 0 ]
   then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      yum -y install $1 
      rpm -q $1 > /dev/null
      if [ $? != 0 ]
      then
         yum -y install $1 
         rpm -q $1 > /dev/null
         if [ $? != 0 ]
         then
            yum -y install $1 
            rpm -q $1 > /dev/null
            if [ $? != 0 ]
            then
               echo "    FAILED to install - $1!" 
               exit 4
            fi
         fi         
      fi
      echo "    DONE."
   fi
}

function gem_install_and_check {
   echo -n "  Checking for (gem package) $1..." 
   gem list | grep $1 > /dev/null
   if [ $? = 0 ]
   then
      echo " FOUND."
   else
      echo " NOT FOUND, installing..."
      gem install $1 
      if [ $? != 0 ]
      then
         gem install $1 
         if [ $? != 0 ]
         then
            gem install $1 --no-rdoc 
            if [ $? != 0 ]
            then
               echo "    FAILED to install - $1!" 
               exit 5
            fi
         fi         
      fi
      echo "    DONE."
   fi
}

function accept_unsigned_yum_packages {
   sed -i -e "s/gpgcheck=1/gpgcheck=0/g" $1
}

#* Disable SELinux (could be done from a GUI install, but just to be sure....)
sed -i -e "s/SELINUX=enabled/SELINUX=disabled/g" /etc/selinux/config

#* Disable the Firewall (could be done from a GUI install, but just to be sure....)
service iptables stop
chkconfig iptables off

#* General update.
yum -y update

#* For packages from SIPfoundry.
if [ ! -f /etc/yum.repos.d/sipxecs-unstable-fc.repo ] 
then
   wget_retry http://sipxecs.sipfoundry.org/temp/sipXecs/sipxecs-unstable-fc.repo /etc/yum.repos.d
fi

#* Tell yum to accept unsigned packages. 
accept_unsigned_yum_packages /etc/yum.conf 
accept_unsigned_yum_packages /etc/yum.repos.d/sipxecs-unstable-fc.repo

#* Install the required packages.
YUM_PACKAGES="gcc gcc-c++ autoconf automake libtool subversion rpm-build httpd httpd-devel openssl-devel jpackage-utils pcre-devel expat-devel unixODBC-devel jakarta-commons-beanutils jakarta-commons-collections jakarta-commons-net ant log4j junit ant-commons-logging ant-trax ant-nodeps postgresql-server zlib-devel postgresql-devel cppunit cppunit-devel redhat-rpm-config alsa-lib-devel curl curl-devel gnutls-devel lzo-devel mysql-devel ncurses-devel python-devel termcap ruby ruby-devel ruby-postgres rubygems rubygem-rake ruby-dbi bind cgicc-devel java-1.6.0-sun-devel java-fonts w3c-libwww-devel xerces-c-devel jain-sip git tftp-server doxygen rpm-build zip which unzip createrepo ant-junit mod_ssl libXp libpng-devel libart_lgpl-devel freetype freetype-devel rpmdevtools alsa-lib-devel curl-devel gnutls-devel lzo-devel gdb gdbm-devel mysql-devel ncurses-devel python-devel termcap dnsjava nsis vsftpd sipx-jasperreports-deps"
for package in $YUM_PACKAGES;
do
   yum_install_and_check $package
done

#* RPMs you can't get via yum.
RPM_PACKAGES="ftp://mirror.switch.ch/pool/1/mirror/epel/5/i386/scons-0.98.1-1.el5.noarch.rpm"
for package in $RPM_PACKAGES;
do
   rpm_file_install_and_check $package
done

#* Ruby gems.
gem update --system
GEM_PACKAGES="file-tail"
for package in $GEM_PACKAGES;
do
   gem_install_and_check $package
done

#* Add the development user, although it may already be done.
useradd $DEVEL_USER
if [ 9 != $? ]; then # Only change the password if the user didn't already exist.
   echo $DEFAULT_PASSWORD | passwd $DEVEL_USER --stdin
fi

#* Give the development user password-less sudo privileges.
sed -i -e "s/# %wheel[\t]ALL=(ALL)[\t]NOPASSWD/%wheel\tALL=(ALL)\tNOPASSWD/g" /etc/sudoers 
usermod -a -G wheel $DEVEL_USER

# Enable TFTP.  Don't bother chaging the /tftpboot, it will later be replaced with a symbolic link
# by the $DEVEL_USER.
sed -i -e "s/[\t]disable[\t][\t][\t]= yes/\tdisable\t\t\t= no/g" /etc/xinetd.d/tftp
/sbin/service xinetd restart

# Enable FTP with a Polycom user, also using the /tftpboot directory.
adduser -d /tftpboot -G $DEVEL_USER -s /sbin/nologin -M PlcmSpIp
echo -e "PlcmSpIp" | sudo passwd --stdin PlcmSpIp
echo "dirlist_enable=NO" >> /etc/vsftpd/vsftpd.conf
chkconfig vsftpd on
service vsftpd restart

# Enable postgresql.
service postgresql initdb
chkconfig postgresql on
service postgresql start

# Get rid of the svn certificate prompt for $DEVEL_USER.  
sudo -u $DEVEL_USER echo p | svn co https://sipxecs.sipfoundry.org/rep/sipXecs/main/sipXcallLib/include/tapi/ /tmp/del_me
rm -rf /tmp/del_me

echo ""
echo "Script complete.  Please reboot."
echo ""

