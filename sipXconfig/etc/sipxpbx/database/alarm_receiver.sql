create table alarm_receiver
(
  alarm_receiver_id integer primary key,
  address varchar(255) not null,
  port int4,
  community_string varchar(255)
);

-- no attempt to migrate data made
drop table alarm_group_snmpcontacts;
create sequence alarm_receiver_seq;
