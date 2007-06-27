-- create schedule table
create table schedule
(
  schedule_id integer not null,
  user_id integer,
  name varchar(255) not null,
  description varchar(255),
  schedule_type char(1) not null,
  constraint schedule_pkey primary key (schedule_id)
);

-- create schedule_hours table
create table schedule_hours
(
  schedule_hours_id integer not null,
  schedule_id integer not null,
  "start" timestamp without time zone,
  "stop" timestamp without time zone,
  "day" varchar(255),
  constraint pk_schedule_hours_id_schedule_id primary key (schedule_hours_id, schedule_id),
  constraint fk_schedule_hours_schedule foreign key (schedule_id)
      references schedule (schedule_id) match full
      on update no action on delete no action
);

-- create sequence fot schedule table
create sequence schedule_seq
  increment 1
  minvalue 1
  maxvalue 9223372036854775807
  start 98
  cache 1;

-- modify ring table to add schedule_id column
alter table ring add schedule_id integer;

-- modify ring table to add foreign key constraint to schedules table
alter table ring add constraint schedule_fkey foreign key (schedule_id) 
	references schedule(schedule_id) match full;