create sequence freeswitch_ext_seq;
create sequence freeswitch_condition_seq;
create sequence freeswitch_action_seq;

create table freeswitch_extension
(
  freeswitch_ext_id integer not null,
  name character varying(255) not null,
  description character varying(255),
  freeswitch_ext_type char(1) not null,
  location_id integer not null,
  primary key (freeswitch_ext_id),
  constraint fk_location foreign key (location_id)
      references location (location_id) match full
);

create table freeswitch_condition
(
  freeswitch_condition_id integer not null,
  field character varying(255) not null,
  expression character varying(255) not null,
  freeswitch_ext_id integer not null,
  primary key (freeswitch_condition_id),
  constraint fk_freeswitch_ext foreign key (freeswitch_ext_id)
      references freeswitch_extension (freeswitch_ext_id) match simple
);

create table freeswitch_action
(
  freeswitch_action_id integer not null,
  application character varying(255) not null,
  data character varying(255),
  freeswitch_condition_id integer not null,
  primary key (freeswitch_action_id),
  constraint fk_freeswitch_condition foreign key (freeswitch_condition_id)
      references freeswitch_condition (freeswitch_condition_id) match simple
);
