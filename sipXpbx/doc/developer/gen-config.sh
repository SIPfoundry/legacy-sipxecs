#!/bin/sh

host=scott.pt.sipit.net
Group=10
TopGroup=99

CfgDir=""

while [ $# -ne 0 ]
do
    case ${1} in
        ##
        ## handle the 'end of options' marker
        ##
        --)
            while [ $# -ne 1 ]
            do
                Uninterpreted="${Uninterpreted} ${2}"
                shift # consume all but one
            done
            ;;

        ##
        ## handle an unknown switch
        ##
        -*)
            Action=USAGE
            break
            ;;

        *)
            if [ -z "${CfgDir}" ]
            then
                CfgDir=${1}
            else
                echo "Too many arguments supplied: $@" 1>&2
                Action=USAGE
                break
            fi
            ;;
    esac

    shift # always consume 1
done

if [ "${Action}" = "USAGE" ]
then
    cat <<USAGE

Usage:

    gen-config <sipx-config-dir>


USAGE
    exit
fi

if [ -z "${CfgDir}" ]
then
  CfgDir=/opt/sipx/var/sipxdata/sipdb
fi

echo -n 'generate groups '

cat /dev/null                                                   > ${CfgDir}/credential.xml
cat /dev/null                                                   > ${CfgDir}/permission.xml
cat /dev/null                                                   > ${CfgDir}/alias.xml

echo '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' >> ${CfgDir}/credential.xml
echo '<items type="credential">'                               >> ${CfgDir}/credential.xml

echo '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' >> ${CfgDir}/permission.xml
echo '<items type="permission">'                               >> ${CfgDir}/permission.xml

echo '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' >> ${CfgDir}/alias.xml
echo '<items type="alias">'                                    >> ${CfgDir}/alias.xml

while [ ${Group} -le ${TopGroup} ]
do
  Prefix=1${Group}

  ua=1
  while [ ${ua} -lt 10 ]
  do
    userNum=$((${Prefix} * 10 + ${ua}))
    user="${userNum}@${host}"

    ################# Credential ################
    if [ `uname -s` = FreeBSD ] ; then
      MD5SUM="/sbin/md5 -r"
    else
      MD5SUM=md5sum
    fi
    token=`echo -n "${userNum}:${host}:1234" | $MD5SUM | cut -d " " -f 1`
    cat <<EOF >> ${CfgDir}/credential.xml
  <item>
    <uri>sip:${user}</uri>
    <realm>${host}</realm>
    <userid>${userNum}</userid>
    <passtoken>${token}</passtoken>
    <authtype>DIGEST</authtype>
  </item>
EOF

    ################# Permission ################
    case ${ua} in
      [12])
        echo '  <item>'                                        >> ${CfgDir}/permission.xml
        echo "    <identity>${user}</identity>"                >> ${CfgDir}/permission.xml
        echo '    <permission>Voicemail</permission>'          >> ${CfgDir}/permission.xml
        echo '  </item>'                                       >> ${CfgDir}/permission.xml
        ;;

      [3456])
        echo '  <item>'                                        >> ${CfgDir}/alias.xml
        echo "    <identity>sip:5${Group}0@${host}</identity>" >> ${CfgDir}/alias.xml
        echo "    <contact>sip:${user}</contact>"              >> ${CfgDir}/alias.xml
        echo '  </item>'                                       >> ${CfgDir}/alias.xml

        echo '  <item>'                                        >> ${CfgDir}/alias.xml
        echo "    <identity>sip:6${Group}0@${host}</identity>" >> ${CfgDir}/alias.xml
        echo "    <contact>sip:${user}</contact>"              >> ${CfgDir}/alias.xml
        echo '  </item>'                                       >> ${CfgDir}/alias.xml
        ;;

      [789])
        echo '  <item>'                                        >> ${CfgDir}/permission.xml
        echo "    <identity>${user}</identity>"                >> ${CfgDir}/permission.xml
        echo '    <permission>BigShot</permission>'            >> ${CfgDir}/permission.xml
        echo '  </item>'                                       >> ${CfgDir}/permission.xml
        ;;
    esac

    ua=$((${ua} + 1))
  done

  test $(($Group % 10)) -eq 0 && echo -n '.'

  Group=$((${Group} + 1))
done

echo '</items>'                                                >> ${CfgDir}/credential.xml
echo '</items>'                                                >> ${CfgDir}/permission.xml
echo '</items>'                                                >> ${CfgDir}/alias.xml

echo ' configuration done.'
