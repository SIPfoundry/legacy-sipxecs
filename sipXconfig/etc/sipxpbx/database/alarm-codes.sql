create sequence alarm_code_seq;

create table alarm_code (
  alarm_seq_id integer not null,
  alarm_code_id varchar(255) not null,
  email_group varchar(255) not null,
  min_threshold integer not null,
  constraint alarm_code_pkey primary key (alarm_seq_id)
);
