create table alarm_server
(
  alarm_server_id integer not null,
  email_notification_enabled boolean,
  constraint alarm_server_pkey primary key (alarm_server_id)
);

create sequence alarm_server_seq;

create table alarm_contacts (
    alarm_server_id integer not null,
    address varchar(255) not null,
    index integer not null,
  	constraint contacts_pkey primary key (alarm_server_id, index),
  	constraint fk_alarm_server_contacts foreign key (alarm_server_id)
  	references alarm_server
);