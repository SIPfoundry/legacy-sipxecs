create table permission (
  permission_id int4 not null,
  description varchar(255),
  label varchar(255),
  default_value boolean not null,
  primary key (permission_id)
);
