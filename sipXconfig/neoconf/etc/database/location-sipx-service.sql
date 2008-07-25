create table location_sipx_service (
  location_id int4 not null,
  sipx_service_id int4 not null,
  primary key (location_id, sipx_service_id)
);

alter table location_sipx_service
  add constraint location_sipx_service_fk1
  foreign key (location_id) references location;
alter table location_sipx_service
  add constraint location_sipx_service_fk2
  foreign key (sipx_service_id) references sipx_service;

insert into initialization_task (name) values ('initialize-location-service-mapping');
