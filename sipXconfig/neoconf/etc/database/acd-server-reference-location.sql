-- modify acd_server table to add location_id column
alter table acd_server add location_id int4;

-- modify acd_server table to add foreign key constraint to location table
alter table acd_server add constraint fk_location_id foreign key (location_id)
	references location(location_id) match full;

insert into initialization_task (name) values ('acd_server_migrate_acd_service');