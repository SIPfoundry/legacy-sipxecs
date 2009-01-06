#!/bin/bash
#
# Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# See http://sipx-wiki.calivia.com/index.php/Express_Development_Environment_Setup for instructions.

# Clean up the old (which may not even exist...)
sudo /sbin/service sipxpbx stop
sudo killall httpd

# You can override the CODE directory, relative to the current directory.
INSTALL=INSTALL
BUILD=BUILD
CODE=main
if [ $# -gt 0 ]
then
   CODE=$1
fi
if [ ! -d $CODE ]; then
   echo "Error: Directory './$CODE' does not exist."
   exit 1
fi
echo INSTALL=`pwd`/$INSTALL > env
echo BUILD=`pwd`/$BUILD >> env
echo CODE=`pwd`/$CODE >> env
sudo rm -rf $INSTALL $BUILD
mkdir $INSTALL
mkdir $BUILD

# Easy scripts to start, stop, and get status.
echo sudo `pwd`/$INSTALL/etc/init.d/sipxpbx start > /tmp/sstart
sudo mv /tmp/sstart /usr/bin/
sudo chmod a+rx /usr/bin/sstart
echo sudo `pwd`/$INSTALL/etc/init.d/sipxpbx stop > /tmp/sstop
sudo mv /tmp/sstop /usr/bin/
sudo chmod a+rx /usr/bin/sstop
echo sudo `pwd`/$INSTALL/etc/init.d/sipxpbx status > /tmp/sstatus
sudo mv /tmp/sstatus /usr/bin/
sudo chmod a+rx /usr/bin/sstatus

# Install FreeSWITCH.
SIPFOUNDRY_RPM_BASE_URL=http://sipxecs.sipfoundry.org/temp/sipXecs/main/FC/8/i386/RPM
function install_sipfoundry_rpm {
   # Out with the old.
   rm -rf $1-*.rpm
   rpm -q $1 > /dev/null
   if [ $? = 0 ]
   then
      sudo rpm --erase --nodeps $1
   fi

   # In with the new.
   rm -rf index.html*
   wget $SIPFOUNDRY_RPM_BASE_URL
   rpm_name=`grep $1-[0-9] index.html | ruby -e 'puts STDIN.readline.split("a href=\"")[1].split("\"")[0]'`
   rm -rf index.html*
   wget $SIPFOUNDRY_RPM_BASE_URL/$rpm_name
   sudo rpm -ihv $rpm_name
   if [ $? != 0 ]
   then
      echo "Error: RPM install failed, see console output."
      exit 2
   fi
   rm -rf $rpm_name
}
install_sipfoundry_rpm sipx-freeswitch

# Configure, compile, test, and install.
pushd $INSTALL
FULL_INSTALL_PATH=`pwd`
popd
pushd $CODE
FULL_CODE_PATH=`pwd`
autoreconf -if  
if [ $? != 0 ]
then
   echo "Error: autoreconf failed, see console output."
   exit 3
fi
popd

pushd $BUILD
$FULL_CODE_PATH/configure --srcdir=$FULL_CODE_PATH --cache-file=`pwd`/ac-cache-file SIPXPBXUSER=`whoami` --prefix=$FULL_INSTALL_PATH --enable-reports --enable-agent --enable-cdr --enable-conference &> configure_output.txt
if [ $? != 0 ]
then
   echo "Error: configure failed, see configure_output.txt."
   exit 4
fi
make recurse TARGETS="all install" &> make_output.txt
if [ $? != 0 ]
then
   echo "Error: make failed, see make_output.txt."
   exit 5
fi
popd

# This is needed so often, we might as well make it easily available with "sudo /sbin/service sipxpbx xxx", 
# and started automatically after reboot.  
sudo rm -rf /etc/init.d/sipxpbx
sudo ln -s $FULL_INSTALL_PATH/etc/init.d/sipxpbx /etc/init.d/sipxpbx

# Adjust the TFTP /FTP directory.
TFTP_PATH=$FULL_INSTALL_PATH/var/sipxdata/configserver/phone/profile/tftproot
ruby -e 'path=""; ARGV[0].split("/").each {|x| path+=x+"/"; `sudo chmod g+x #{path}`}' $TFTP_PATH
sudo rm -rf /tftpboot
sudo ln -s $TFTP_PATH /tftpboot

# Clear any database contents that might be left over from the last install.
$INSTALL/bin/sipxconfig.sh --database drop create &> sipxconfig_drop_create.log
$INSTALL/bin/sipxconfig.sh --first-run &> sipxconfig_first-run.log

# Fix FreeSWITCH
sudo $INSTALL/bin/freeswitch.sh --configtest &> freeswitch_configtest.log

# sipx-setup
sudo $INSTALL/bin/sipx-setup

# Show whether or not the services got started by the above.  (It is an option...)
sudo /sbin/service sipxpbx status  
if [ $? != 0 ]
then
   echo ""
   echo "Starting sipXecs...."
   sstart
   sstatus
fi

echo ""
echo "TO START: sstart"
echo "TO STOP : sstop"
echo "TO GET STATUS : sstatus"
echo ""
echo "ENV:"
cat env
echo ""
echo "DONE!"


