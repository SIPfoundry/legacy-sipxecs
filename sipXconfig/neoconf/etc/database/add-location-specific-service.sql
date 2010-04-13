create table location_specific_service(
  location_specific_service_id int4 not null primary key,
  location_id int4 not null,
  sipx_service_id int4 not null,
  enable_on_next_upgrade boolean not null default false,
  constraint fk_location_id
    foreign key (location_id)
	references location,
  constraint fk_sipx_service
    foreign key (sipx_service_id)
    references sipx_service
);

create sequence location_specific_service_seq;

-- copy services from old table
insert into location_specific_service(
  location_specific_service_id, location_id, sipx_service_id)
  select nextval('location_specific_service_seq'),
    location_id,sipx_service_id from location_sipx_service;

-- remove table that is no longer used
drop table location_sipx_service cascade;
