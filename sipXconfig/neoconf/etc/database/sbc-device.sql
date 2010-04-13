create table sbc_device (
   sbc_device_id int4 not null,
   bean_id varchar(255) not null,
   name varchar(255) unique,
   address varchar(255),
   description varchar(255),
   serial_number varchar(255),
   value_storage_id int4,
   model_id varchar(64) not null,
   device_version_id varchar(32),
   primary key (sbc_device_id)
);
