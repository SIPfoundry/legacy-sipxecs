create table upload(
  upload_id int4 not null primary key,
  name varchar(255) not null unique,
  bean_id varchar(32) not null,
  specification_id varchar(32),
  value_storage_id int4,
  description varchar(255)
);

create sequence upload_seq;

alter table upload add constraint upload_value_storage foreign key (value_storage_id) references value_storage;
