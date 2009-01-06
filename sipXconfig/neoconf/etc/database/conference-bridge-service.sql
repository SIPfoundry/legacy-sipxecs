-- Remove the now-redundant properties (these will be taken from the associated service)
alter table meetme_bridge drop host;
alter table meetme_bridge drop port;
alter table meetme_bridge drop name;
alter table meetme_bridge drop description;

alter table meetme_bridge add location_specific_service_id integer;
alter table meetme_bridge add constraint fk_location_specific_service_id 
    foreign key(location_specific_service_id) references location_specific_service;

-- Remove existing conferences and bridges in preparation for migration
delete from meetme_conference;
delete from meetme_bridge;
    
-- Set up the migration task
insert into initialization_task (name) values ('migrate-conference-bridges');    
