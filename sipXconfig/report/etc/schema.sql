/* suppress warnings */
SET client_min_messages TO 'error';

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
