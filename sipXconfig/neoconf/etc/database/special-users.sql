create table special_user (
  special_user_id int4 not null,
  user_type varchar(255),
  sip_password varchar(255),
  primary key (special_user_id)
);
