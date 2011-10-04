ALTER TABLE location ADD state varchar(32) not null default 'UNCONFIGURED';
ALTER TABLE location ADD last_attempt timestamp;

create table location_failed_replications (
  location_id int4 not null,
  entity_name varchar(255) not null,
  primary key (location_id, entity_name)
);

alter table location_failed_replications
  add constraint location_failed_replications_fk
  foreign key (location_id) references location;