create table location_bundle (
  location_id int4 not null,
  bundle_bean varchar(255) not null,
  primary key (location_id, bundle_bean)
);

alter table location_bundle
  add constraint location_bundle_fk
  foreign key (location_id) references location;
