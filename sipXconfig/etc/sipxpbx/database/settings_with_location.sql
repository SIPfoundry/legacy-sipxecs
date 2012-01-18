create sequence settings_with_location_seq;

-- drop table settings_with_location;
create table settings_with_location (
  settings_with_location_id integer not null primary key,
  bean_id varchar(255) not null,
  location_id integer not null,
  value_storage_id integer
);

-- drop table settings_location_group;
create table settings_location_group (
  settings_with_location_id integer not null,
  group_id integer not null,
  primary key(settings_with_location_id, group_id)
);

-- alter table settings_with_location drop constraint settings_with_location_location_id;
alter table settings_with_location
  add constraint settings_with_location_location_id 
    foreign key (location_id) references location;

-- alter table settings_with_location drop constraint settings_with_location_value_storage_id;
alter table settings_with_location 
  add constraint settings_with_location_value_storage_id 
    foreign key (value_storage_id) references value_storage;

-- alter table settings_location_group drop constraint settings_location_group_group_id;
alter table settings_location_group 
  add constraint settings_location_group_group_id
    foreign key (group_id) references group_storage;

-- alter table settings_location_group drop constraint settings_location_group_settings_with_location_id;
alter table settings_location_group 
  add constraint settings_location_group_settings_with_location_id
    foreign key (settings_with_location_id) references settings_with_location;
    