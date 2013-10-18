create sequence bean_with_settings_seq;

-- drop table bean_with_settings;
create table bean_with_settings (
  bean_with_settings_id integer not null primary key,
  bean_id varchar(255) not null,
  value_storage_id integer
);

alter table bean_with_settings 
  add constraint bean_with_settings_value_storage_id
    foreign key (value_storage_id) references value_storage;

