/* suppress warnings */
SET client_min_messages TO 'error';

/**
 * CHANGE VERSION HERE ----------------------------> X <------------------
 *
 * To add new patch:
 *  - create a new SQL file i.e. patch.sql
 *  - modify UpgradePatchesN variable in sipxconfig-report-setup to install new patch
 *
 * To roll in patches into schema:
 *  - add SQL corresponding to the patch in this file
 *  - update version number inserted in version_history table below
 *  - update DbVersion variable in sipxconfig-report-setup
 *  - do *not* delete patch file
 * For the initial sipX release with ACD History, the database version is 1.
 * Version 2: location_fqdn new column
 */

create table acd_agent_stat (
  acd_agent_stat_id serial8 not null primary key,
  agent_uri text not null,
  queue_uri text,
  sign_in_time timestamp not null,
  sign_out_time timestamp
);

create table acd_call_stat (
  acd_call_stat_id serial8 not null primary key,
  from_uri text not null,
  state varchar(32) not null,
  agent_uri text,
  queue_uri text not null,
  enter_time timestamp,
  pick_up_time timestamp,
  terminate_time timestamp
);

create table bird (
  bird_id serial8 not null primary key,
  species varchar(256)
);
