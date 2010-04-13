create sequence alarm_group_seq;

create table alarm_group (
  alarm_group_id integer not null,
  name varchar(255) not null,
  description character varying(255),
  enabled boolean not null default false,
  constraint alarm_group_pkey primary key (alarm_group_id)
);
insert into alarm_group (alarm_group_id, name, description, enabled) values (nextval('alarm_group_seq'), 'default', 'Default alarm group', true);

create table alarm_group_smscontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  index integer not null,
  constraint group_smscontacts_pkey primary key (alarm_group_id, index)
);

create table alarm_group_emailcontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  index integer not null,
  constraint group_emailcontacts_pkey primary key (alarm_group_id, index)
);

create table user_alarm_group
(
  alarm_group_id integer not null,
  user_id integer not null,
  constraint user_alarm_group_pkey primary key (alarm_group_id, user_id),
  constraint alarm_group_fk1 foreign key (alarm_group_id)
      references alarm_group (alarm_group_id) match simple
      on update no action on delete no action,
  constraint alarm_group_fk2 foreign key (user_id)
      references users (user_id) match simple
      on update no action on delete no action
);
