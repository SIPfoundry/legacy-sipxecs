#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# sipxinterop-setup.in.sh

# Allows the changes to be undone with sipxinterop-revert.in.sh
if [ -d ~/interop-revert ]; then
   echo "Keeping existing ~/interop-revert..."
else
   rm -rf ~/interop-revert
   mkdir ~/interop-revert

   mkdir ~/interop-revert/process.d
   cp @SIPX_DATADIR@/process.d/* ~/interop-revert/process.d

   mkdir ~/interop-revert/sipdb
   cp @SIPX_DBDIR@/* ~/interop-revert/sipdb

   mkdir ~/interop-revert/homedir
   cp @SIPX_CONFDIR@/*.xml ~/interop-revert/homedir
   cp @SIPX_CONFDIR@/*.conf ~/interop-revert/homedir
   cp @SIPX_CONFDIR@/*-config ~/interop-revert/homedir

   mkdir ~/interop-revert/http_rootdir
   cp @HTTP_ROOTDIR@/*.* ~/interop-revert/http_rootdir
fi

INTEROP_OUT_DIR="/tmp/interop-out"

PASSWORD="1234"

function pre_compare { # filename
   FILENAME=`ruby -e 'puts File.basename(ARGV[0])' $1`
   cp $1 /tmp/ORIG_$FILENAME
}

function post_compare { # filename, index
   FILENAME=`ruby -e 'puts File.basename(ARGV[0])' $1`
   diff $1 /tmp/ORIG_$FILENAME > /dev/null
   if [ $? == 0 ]; then
      echo "ERROR: Failed to edit $1! ($2)" >&2
      exit 1
   fi
}

function process_substitutions { # file
   sed -i \
      -e 's/@SIPXCHANGE_DOMAIN_NAME@/'"$SIPXCHANGE_DOMAIN_NAME"'/g' \
      $1
}

function return_digest_A1 { # username, realm, password
   username=$1
   realm=$2
   password=$3
   A1=`echo -n "$username:$realm:$password"|md5sum|cut -d" " -f1`
   echo $A1
}

echo SIPX_DATADIR - @SIPX_DATADIR@
echo SERVICEDIR - @SERVICEDIR@
echo SIPX_DBDIR - @SIPX_DBDIR@
echo SIPX_CONFDIR - @SIPX_CONFDIR@
echo ""

# Determine the SIP Realm.
SIPXCHANGE_DOMAIN_NAME=`grep SIPX_PROXY_AUTHENTICATE_REALM @SIPX_CONFDIR@/sipXproxy-config | ruby -e 'puts STDIN.readline.split(":")[1].strip'`
echo SIPXCHANGE_DOMAIN_NAME - $SIPXCHANGE_DOMAIN_NAME
echo ""

# Process the part files.
PART_DIR=$INTEROP_OUT_DIR/parts
PART_FILES=`ls $PART_DIR/*.pre`
for filename in $PART_FILES; do
   new_filename=`ruby -e 'puts ARGV[0].sub(".pre", "")' $filename`
   cp $filename $new_filename
   process_substitutions $new_filename
done

# Stop sipXecs
@SERVICEDIR@/sipxecs stop
if [ $? != 0 ]; then
   echo "ERROR: Failed to stop sipXecs!" >&2
   exit 2
fi

# Disable all services.
ALL_SERVICES=`ls @SIPX_DATADIR@/process.d/*-process.xml`
for service in $ALL_SERVICES; do
   mv $service $service-DISABLED
done

# Enable only the required services. (sipxrelay is required for sipXproxy.)
INTEROP_SERVICES="sipxsupervisor sipXproxy sipxrelay sipXregistry sipxrls sipxpark freeswitch sipxivr sipxacd sipXvxml sipxpresence sipstatus"
for interop_service in $INTEROP_SERVICES; do
   mv @SIPX_DATADIR@/process.d/$interop_service-process.xml-DISABLED @SIPX_DATADIR@/process.d/$interop_service-process.xml
   if [ $? != 0 ]; then
      echo "ERROR: Failed to re-enable $interop_service!" >&2
      exit 3
   fi   
done

function generate_credential_user { # username, realm, passtoken_password, pintoken_password, file
   username=$1
   realm=$2
   passtoken_password=$3
   pintoken_password=$4
   out_file=$5
   echo "    <item>" >> $out_file
   echo "        <realm>$realm</realm>" >> $out_file
   echo "        <uri>sip:$username@$realm</uri>" >> $out_file
   echo "        <userid>$username</userid>" >> $out_file
   A1=$(return_digest_A1 $username $realm $passtoken_password)
   echo "        <passtoken>$A1</passtoken>" >> $out_file
   A1=$(return_digest_A1 $username $realm $pintoken_password)
   echo "        <pintoken>$A1</pintoken>" >> $out_file
   echo "        <authtype>DIGEST</authtype>" >> $out_file
   echo "    </item>" >> $out_file
}

function return_superadmin_password {
   SUPERADMIN_PASSWORD=`psql -U postgres -d SIPXCONFIG -c "select * from users where user_name='superadmin'" | tail -3 | head -1 | ruby -e 'puts STDIN.readline.split(" ")[5]'`
   if [ $SUPERADMIN_PASSWORD == "nil" ]; then
      echo "ERROR: Failed to extract the superadmin password!" >&2
      exit 4
   fi
   echo $SUPERADMIN_PASSWORD
}

# @SIPX_DBDIR@/credential.xml - Generate from scratch (pull special & superadmin passwords from the DB)
echo "<?xml version=\"1.0\" standalone=\"yes\" ?>" > @SIPX_DBDIR@/credential.xml
echo "<items type=\"credential\" xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/credential-00-00\">" >> @SIPX_DBDIR@/credential.xml
generate_credential_user "superadmin" $SIPXCHANGE_DOMAIN_NAME $(return_superadmin_password) DOESNOTMATTER @SIPX_DBDIR@/credential.xml
for gg in {10..99}; do # The 89 Groups
   for y in {1..9}; do # The 9 Users per Group
      generate_credential_user 1$gg$y $SIPXCHANGE_DOMAIN_NAME $PASSWORD $PASSWORD @SIPX_DBDIR@/credential.xml
      echo -n "."
   done
done
echo ""
INTEROP_SPECIAL_IDS="@INTEROP_SPECIAL_IDS@"
for special_id in $INTEROP_SPECIAL_IDS; do
   SPECIAL_USERNAME=`ruby -e 'puts ARGV[0].split("/")[0]' $special_id`
   SPECIAL_USERTYPE=`ruby -e 'puts ARGV[0].split("/")[1]' $special_id`
   SPECIAL_PASSWORD=`psql -U postgres -d SIPXCONFIG -c "select * from special_user where user_type='$SPECIAL_USERTYPE'" | tail -3 | head -1 | ruby -e 'puts STDIN.readline.split(" ")[4]'`
   echo "    <item>" >> @SIPX_DBDIR@/credential.xml
   echo "        <realm>$SIPXCHANGE_DOMAIN_NAME</realm>" >> @SIPX_DBDIR@/credential.xml
   echo "        <uri>sip:$SPECIAL_USERNAME@$SIPXCHANGE_DOMAIN_NAME</uri>" >> @SIPX_DBDIR@/credential.xml
   echo "        <userid>$SPECIAL_USERNAME </userid>" >> @SIPX_DBDIR@/credential.xml
   A1=$(return_digest_A1 $SPECIAL_USERNAME $SIPXCHANGE_DOMAIN_NAME $SPECIAL_PASSWORD)
   echo "        <passtoken>$A1</passtoken>" >> @SIPX_DBDIR@/credential.xml
   echo "        <pintoken></pintoken>" >> @SIPX_DBDIR@/credential.xml
   echo "        <authtype>DIGEST</authtype>" >> @SIPX_DBDIR@/credential.xml
   echo "    </item>" >> @SIPX_DBDIR@/credential.xml
done
echo "</items>" >> @SIPX_DBDIR@/credential.xml

# @SIPX_DBDIR@/permission.xml - Generate from scratch
echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<items type=\"permission\" xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/permission-00-00\">" > @SIPX_DBDIR@/permission.xml
for gg in {10..99}; do # The 89 Groups
   for y in {1..2}; do # Users 1gg1 and 1gg2 have Voicemail
      echo "   <item>
    <identity>1$gg$y@$SIPXCHANGE_DOMAIN_NAME</identity>
    <permission>Voicemail</permission>
  </item>
  <item>
    <identity>1$gg$y@$SIPXCHANGE_DOMAIN_NAME</identity>
    <permission>SipXVoicemailServer</permission>
  </item>
  <item>
    <identity>~~vm~1$gg$y@$SIPXCHANGE_DOMAIN_NAME</identity>
    <permission>SipXVoicemailServer</permission>
  </item>" >> @SIPX_DBDIR@/permission.xml
   done
   for y in {7..9}; do # Users 1gg7, 1gg8, and 1gg9 have BigShot
      echo "  <item>
    <identity>1$gg$y@$SIPXCHANGE_DOMAIN_NAME</identity>
    <permission>BigShot</permission>
  </item>" >> @SIPX_DBDIR@/permission.xml
   done
done
echo "</items>" >> @SIPX_DBDIR@/permission.xml

# @SIPX_DBDIR@/alias.xml - Generate from scratch
echo "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<items type=\"alias\" xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/alias-00-00\">" > @SIPX_DBDIR@/alias.xml
for gg in {10..99}; do # The 89 Groups
   echo "
   <!-- serial fork 5${gg}0 -> 1${gg}3, 1${gg}4, 1${gg}5, 1${gg}6 -->" >> @SIPX_DBDIR@/alias.xml
   for e in 3 4 5 6; do 
      echo "   <item> 
      <identity>5${gg}0@$SIPXCHANGE_DOMAIN_NAME</identity> 
      <contact>&lt;1${gg}${e}@$SIPXCHANGE_DOMAIN_NAME&gt;;q=0.$((8 - ${e}))</contact>
   </item>" >> @SIPX_DBDIR@/alias.xml
   done
   echo "   <!-- parallel fork 6${gg}0 -> 1${gg}3, 1${gg}4, 1${gg}5, 1${gg}6 -->" >> @SIPX_DBDIR@/alias.xml
   for e in 3 4 5 6; do 
      echo "   <item> 
      <identity>6${gg}0@$SIPXCHANGE_DOMAIN_NAME</identity> 
      <contact>&lt;1${gg}${e}@$SIPXCHANGE_DOMAIN_NAME&gt;</contact>
   </item>" >> @SIPX_DBDIR@/alias.xml
   done
done
echo "
   <!-- a series of translations that go over the 20-hop limit -->
      <item>
         <identity>9000@$SIPXCHANGE_DOMAIN_NAME</identity>
         <contact>sip:9000_1@$SIPXCHANGE_DOMAIN_NAME</contact>
      </item>" >> @SIPX_DBDIR@/alias.xml
for x in {1..21}; do # 21 hops
   next=`expr $x + 1`
   echo "
      <item>
         <identity>9000_$x@$SIPXCHANGE_DOMAIN_NAME</identity>
         <contact>sip:9000_$next@$SIPXCHANGE_DOMAIN_NAME</contact>
      </item>" >> @SIPX_DBDIR@/alias.xml
done
echo "</items>" >> @SIPX_DBDIR@/alias.xml

# Fix all the @SIPX_DBDIR@ permissions.
chown @SIPXPBXUSER@ @SIPX_DBDIR@/*
chgrp @SIPXPBXGROUP@ @SIPX_DBDIR@/*

# @SIPX_CONFDIR@/resource-lists.xml - Generate from scratch
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<lists xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01\">" > @SIPX_CONFDIR@/resource-lists.xml
for gg in {10..99}; do # The 89 Groups 
   # 1gg2, 1gg6, and 1gg9 are in the Resource List.
   echo "<list user=\"~~rl~F~${gg}\" user-cons=\"~~rl~C~${gg}\">
    <name>1${gg}n</name>" >> @SIPX_CONFDIR@/resource-lists.xml
   for n in 2 6 9; do 
      echo "    <resource uri=\"sip:1${gg}${n}@$SIPXCHANGE_DOMAIN_NAME;sipx-noroute=VoiceMail;sipx-userforward=false\">
      <name>1${gg}${n}</name>
    </resource>" >> @SIPX_CONFDIR@/resource-lists.xml
   done
   echo "</list>" >> @SIPX_CONFDIR@/resource-lists.xml
done
echo "</lists>" >> @SIPX_CONFDIR@/resource-lists.xml


function generate_validusers_user { # username, realm, pintoken_password, file
   username=$1
   realm=$2
   pintoken_password=$3
   out_file=$4
   echo "  <user>" >> $out_file
   echo "    <identity>$username@$realm</identity>" >> $out_file
   echo "    <userName>$username</userName>" >> $out_file
   echo "    <aliases/>" >> $out_file
   echo "    <displayName>$username</displayName>" >> $out_file
   echo "    <contact>\"$username\"&lt;sip:$username@$realm&gt;</contact>" >> $out_file
   A1=$(return_digest_A1 $username $realm $pintoken_password)
   echo "    <pintoken>$A1</pintoken>" >> $out_file
   echo "    <inDirectory>false</inDirectory>" >> $out_file
   echo "  </user>" >> $out_file
}

# @SIPX_CONFDIR@/orbits.xml - Modify, by inserting a big section generated from scratch
PARK_BACKGROUND_AUDIO=`grep background-audio @SIPX_CONFDIR@/orbits.xml  | head -1`
cp /dev/null $PART_DIR/orbits-added.part
for gg in {10..99}; do # The 89 Groups
   for y in {1..9}; do # The 9 Orbits per Group
      echo "  <orbit>" >> $PART_DIR/orbits-added.part
      echo "    <name>5$gg$y</name>" >> $PART_DIR/orbits-added.part
      echo "    <extension>5$gg$y</extension>" >> $PART_DIR/orbits-added.part
      echo "    "$PARK_BACKGROUND_AUDIO >> $PART_DIR/orbits-added.part
      echo "    <description>Parking orbit 5$gg$y</description>" >> $PART_DIR/orbits-added.part
      echo " </orbit>" >> $PART_DIR/orbits-added.part
   done
done
pre_compare @SIPX_CONFDIR@/orbits.xml
sed -i -e '/<\/music-on-hold>/ r '"$PART_DIR/orbits-added.part"'' @SIPX_CONFDIR@/orbits.xml
post_compare @SIPX_CONFDIR@/orbits.xml 1

# @SIPX_CONFDIR@/validusers.xml - Modify, by inserting a big section generated from scratch
cp /dev/null $PART_DIR/validusers-added.part
generate_validusers_user "superadmin" $SIPXCHANGE_DOMAIN_NAME $(return_superadmin_password) $PART_DIR/validusers-added.part
for gg in {10..99}; do # The 89 Groups
   for y in {1..9}; do # The 9 Users per Group
      generate_validusers_user 1$gg$y $SIPXCHANGE_DOMAIN_NAME $PASSWORD $PART_DIR/validusers-added.part
      echo -n "."
   done
done
echo ""
pre_compare @SIPX_CONFDIR@/validusers.xml
sed -i -e '/validusers-00-00\">/ r '"$PART_DIR/validusers-added.part"'' @SIPX_CONFDIR@/validusers.xml
post_compare @SIPX_CONFDIR@/validusers.xml 1

# @SIPX_CONFDIR@/mappingrules.xml - Modify
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e 's/8x/888&/g' @SIPX_CONFDIR@/mappingrules.xml # Disable the original 8xxx direct VM dialing rule
post_compare @SIPX_CONFDIR@/mappingrules.xml 1
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e 's/<userPattern>\.<\/userPattern>/<userPattern>1xxx<\/userPattern>/g' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 2
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e 's/<user>~~vm~{user}<\/user>/<user>~~vm~{digits}<\/user>/g' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 3
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e 's/<\/hostMatch>/MAPPINGRULES-ADDED-AFTER\n  &/g' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 4
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e '/MAPPINGRULES-ADDED-AFTER/ r '"$PART_DIR/mappingrules-added.part"'' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 5
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e 's/MAPPINGRULES-ADDED-AFTER//g' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 6
pre_compare @SIPX_CONFDIR@/mappingrules.xml
sed -i -e '/urlmap-00-00\">/ r '"$PART_DIR/mappingrules-srvaliases.part"'' @SIPX_CONFDIR@/mappingrules.xml
post_compare @SIPX_CONFDIR@/mappingrules.xml 7

# @SIPX_CONFDIR@/forwardingrules.xml - Modify
pre_compare @SIPX_CONFDIR@/forwardingrules.xml
sed -i -e '/<\/description>/ r '"$PART_DIR/forwardingrules-srvaliases.part"'' @SIPX_CONFDIR@/forwardingrules.xml
post_compare @SIPX_CONFDIR@/forwardingrules.xml 1

# @SIPX_CONFDIR@/authrules.xml - Modify
pre_compare @SIPX_CONFDIR@/authrules.xml
sed -i -e '/urlauth-00-00\">/ r '"$PART_DIR/authrules-added.part"'' @SIPX_CONFDIR@/authrules.xml
post_compare @SIPX_CONFDIR@/authrules.xml 1

# Fix all the @SIPX_CONFDIR@ XML permissions.
chown @SIPXPBXUSER@ @SIPX_CONFDIR@/*.xml
chgrp @SIPXPBXGROUP@ @SIPX_CONFDIR@/*.xml

# Allow cgi-bin program execution.
BACKUP_SUFFIX=".BEGORE"
sed -i$BACKUP_SUFFIX -e 's/Options Indexes FollowSymLinks MultiViews/& ExecCGI/g' @SIPX_CONFDIR@/httpd.conf
diff @SIPX_CONFDIR@/httpd.conf @SIPX_CONFDIR@/httpd.conf$BACKUP_SUFFIX > /dev/null
if [ $? == 0 ]; then
   echo "ERROR: Failed to add ExecCGI to @SIPX_CONFDIR@/httpd.conf!" >&2
   exit 5
fi
sed -i$BACKUP_SUFFIX -e 's/#AddHandler cgi-script/AddHandler cgi-script/g' @SIPX_CONFDIR@/httpd.conf
diff @SIPX_CONFDIR@/httpd.conf @SIPX_CONFDIR@/httpd.conf$BACKUP_SUFFIX > /dev/null
if [ $? == 0 ]; then
   echo "ERROR: Failed to enable AddHandler cgi-script in @SIPX_CONFDIR@/httpd.conf!" >&2
   exit 6
fi

# Bump up the log levels required for tracking SIP messges.
SIP_LOGGING_SERVICES="registrar-config/SIP_REGISTRAR_LOG_LEVEL sipxpark-config/SIP_PARK_LOG_LEVEL sipXproxy-config/SIPX_PROXY_LOG_LEVEL sipxrls-config/SIP_RLS_LOG_LEVEL status-config/SIP_STATUS_LOG_LEVEL"
for service in $SIP_LOGGING_SERVICES; do
   service_filename=`ruby -e 'puts ARGV[0].split("/")[0]' $service`
   service_log=`ruby -e 'puts ARGV[0].split("/")[1]' $service`
   pre_compare @SIPX_CONFDIR@/$service_filename
   sed -i -e 's/'"$service_log"'/##SeeEndOfFile##&/g' @SIPX_CONFDIR@/$service_filename
   post_compare @SIPX_CONFDIR@/$service_filename 1
   echo "$service_log : DEBUG" >> @SIPX_CONFDIR@/$service_filename
done

# Make a couple of the test cases progress a little faster.
pre_compare @SIPX_CONFDIR@/sipXproxy-config
sed -i -e 's/SIPX_PROXY_DEFAULT_SERIAL_EXPIRES/##SeeEndOfFile##&/g' @SIPX_CONFDIR@/sipXproxy-config
post_compare @SIPX_CONFDIR@/sipXproxy-config SIPX_PROXY_DEFAULT_SERIAL_EXPIRES
echo "SIPX_PROXY_DEFAULT_SERIAL_EXPIRES : @INTEROP_DEFAULT_SERIAL_EXPIRES@" >> @SIPX_CONFDIR@/sipXproxy-config

# Add the HTML cna CGI content to @HTTP_ROOTDIR@.
SRC_HTTPD_ROOTDIR="$INTEROP_OUT_DIR/http_rootdir"
HTTPD_ROOTDIR_FILES=`ls $SRC_HTTPD_ROOTDIR`
for filename in $HTTPD_ROOTDIR_FILES; do
   new_filename=`ruby -e 'puts ARGV[0].sub(".pre", "")' $filename`
   cp -f $SRC_HTTPD_ROOTDIR/$filename @HTTP_ROOTDIR@/$new_filename
   process_substitutions @HTTP_ROOTDIR@/$new_filename
done

# Start sipXecs
@SERVICEDIR@/sipxecs start
if [ $? != 0 ]; then
   echo "ERROR: Failed to start sipXecs!" >&2
   exit 7
fi




