create table timezone
(
  timezone_id integer not null,
  gmt_offset integer,
  dstsavings_enabled boolean,
  dst_savings integer,
  start_month integer,
  start_week integer,
  start_day_of_week integer,
  start_time integer,
  stop_month integer,
  stop_week integer,
  stop_day_of_week integer,
  stop_time integer,
  constraint timezone_pkey primary key (timezone_id)
);

create sequence timezone_seq;

insert into initialization_task (name) values ('default-time-zone');
