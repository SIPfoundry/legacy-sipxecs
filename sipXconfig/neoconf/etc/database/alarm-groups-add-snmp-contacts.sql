create table alarm_group_snmpcontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  port int4,
  community_string varchar(255),
  index integer,
  constraint group_snmpcontacts_pkey primary key (alarm_group_id, index)
);

alter table alarm_server rename column email_notification_enabled to alarm_notification_enabled;