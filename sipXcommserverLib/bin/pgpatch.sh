#!/bin/bash
#
# Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.

Action=RUN

: ${ServiceDir:=/etc/init.d}
: ${Chown:=chown}

# This function determines the correct service name for Postgres.
postgresService() {
    # If the user has already specified $Service, do not modify it.
    if test -n "$POSTGRES_SERVICE"
    then
        echo -e "$POSTGRES_SERVICE"
    fi
    
    if [ -f /etc/init.d/rhdb ]
    then
	    # Red Hat Enterprise uses the name rhdb.
        echo -e rhdb
    fi
    
    # Most other distributions use the name postgresql.
    echo -e postgresql
}

# Configure postgres to accept TCP connections for communication
# from Java
postgresSetup() {

  # Set up the server.
  Service=`postgresService`

  # May not by running, so eat up error (ENG-314)
  ${ServiceDir}/${Service} stop 2>&1 1> /dev/null

  # Custom
  if test -z $PGDATA
  then
      # Debian/Gentoo
      if test -d /var/lib/postgresql/data
      then
          PGDATA=/var/lib/postgresql/data
      else
          # Redhat
          PGDATA=/var/lib/pgsql/data
      fi
  fi

  # Postgres db is initialized on startup on Redhat, but not on other
  # distros so unless we put a "if distro=rh"  we need to init here
  if [ ! -f $PGDATA/PG_VERSION ] || [ ! -d $PGDATA/base ]
  then
      $SubstituteUser - postgres -c "initdb --pgdata=$PGDATA"
  fi

  # Create backup file (possibly) requiring update
  if [ ! -f $PGDATA/pg_hba.conf-sipx.bak ]
  then
     cp $PGDATA/pg_hba.conf $PGDATA/pg_hba.conf.sipx.bak
  fi

  # Will allow this script to add user.  Needs to be listed before
  # other permission or it will not take effect.
  if ! grep '^local *all *all *trust\b*$' $PGDATA/pg_hba.conf >/dev/null
  then
     echo "local all all trust" > $PGDATA/pg_hba.conf.tmp
     cat $PGDATA/pg_hba.conf >> $PGDATA/pg_hba.conf.tmp
     mv $PGDATA/pg_hba.conf.tmp $PGDATA/pg_hba.conf
  fi

  # Will allow jdbc to connect.  Needs to be listed before
  # other permission or it will not take effect.
  if ! grep '^host *all *all *127.0.0.1 *255.255.255.255 *trust\b*$' $PGDATA/pg_hba.conf >/dev/null
  then
     echo "host all all 127.0.0.1 255.255.255.255 trust" > $PGDATA/pg_hba.conf.tmp
     cat $PGDATA/pg_hba.conf >> $PGDATA/pg_hba.conf.tmp
     mv $PGDATA/pg_hba.conf.tmp $PGDATA/pg_hba.conf
  fi

  # Open up TCP/IP connections
  sed -i-sipx.bak -e 's/\#tcpip_socket\s=\sfalse/tcpip_socket = true/g' \
          $PGDATA/postgresql.conf
  ${Chown} postgres:postgres $PGDATA/postgresql.conf

  # Postmaster to allow connections
  echo "-i" > $PGDATA/postmaster.opts.default
  chmod 664 $PGDATA/postmaster.opts.default
  ${Chown} postgres:postgres $PGDATA/postmaster.opts.default

  ${ServiceDir}/$Service start
}

# Have postgres start automatically with system reboot
setPostgresRunlevels() {
    # Arrange for Postgres to be started automatically in runlevels 3
    # and 5.
    # Check if we can use chkconfig.
    if [ -f /sbin/chkconfig ]
    then
        # We have to specify the runlevels because the default set of
        # runlevels for Postgres is empty.
        /sbin/chkconfig --level 35 $Service on
    else
        # The user will have to do it manually.
        echo chkconfig does not exist.
        echo You need to create /etc/rc?.d/{S85,K15}$Service
        echo to start postgres for runlevels 3 and 5.
    fi
}


while [ $# -ne 0 ]
do
    case ${1} in
        -h|--help|*)
            Action=HELP
            ;;
    esac           

    shift # always consume 1
done

if [ ${Action} = RUN ]
then
  postgresSetup
  setPostgresRunlevels
elif [ ${Action} = HELP ]
then
cat <<USAGE
Usage: pgpatch.sh [-h|--help]
                     
Patches PostgresSQL cinfuguration file to initialize postgresql for communicating 
with sipxconfig and sipxproxy and create initial database. Will most likely need 
root permissions.

Notable environment variables:

    POSTGRES_SERVICE   a guess is made to determine the name for the
                       Postgres service.
                       If the guess is incorrect, then set this to the name of
                       the script in /etc/init.d that starts/stops
                       the Postgres database.  The possibilities that
                       we are aware of are "postgresql" and "rhdb".

USAGE

fi
