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
 * For the initial sipX release with CDR, the database version is 1.
 * Version 2: location_fqdn new column
 */

create table call_state_events
(
  id bigserial not null,
  observer text not null,
  event_seq bigint not null,
  event_time timestamp without time zone not null,
  event_type character(1) not null,
  cseq integer not null,
  call_id text not null,
  from_tag text,
  to_tag text,
  from_url text not null,
  to_url text not null,
  contact text,
  refer_to text,
  referred_by text,
  failure_status smallint,
  failure_reason text,
  request_uri text,
  reference text,
  caller_internal boolean,
  callee_route text,
  constraint call_state_events_pkey primary key (id)
);

create table cdrs
(
  id bigserial not null,
  call_id text not null,
  from_tag text not null,
  to_tag text not null,
  caller_aor text not null,
  callee_aor text not null,
  start_time timestamp without time zone,
  connect_time timestamp without time zone,
  end_time timestamp without time zone,
  termination character(1),
  failure_status smallint,
  failure_reason text,
  call_direction character(1),
  reference text,
  caller_contact text,
  callee_contact text,
  caller_internal boolean,
  callee_route text,
  constraint cdrs_pkey primary key (id),
  constraint cdrs_call_id_unique unique (call_id)
);

create table observer_state_events
(
  id bigserial not null,
  observer text not null,
  event_seq bigint not null,
  event_time timestamp without time zone not null,
  status smallint not null,
  msg text,
  constraint observer_state_events_pkey primary key (id)
)