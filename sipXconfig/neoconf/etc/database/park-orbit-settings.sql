alter table park_orbit add column value_storage_id int4;
alter table park_orbit add column group_id int4;

create table park_orbit_group (
   park_orbit_id int4 not null,
   group_id int4 not null,
   primary key (park_orbit_id, group_id)
);
