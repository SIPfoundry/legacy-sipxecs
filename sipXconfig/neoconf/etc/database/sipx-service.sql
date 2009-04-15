create table sipx_service(
  sipx_service_id int4 not null primary key,
  bean_id varchar(32) not null,
  value_storage_id int4
);

create sequence sipx_service_seq;