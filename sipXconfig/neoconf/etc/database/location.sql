create table location (
	location_id int4 not null,
	name varchar(255) not null,
	process_monitor_url varchar(255),
    replication_url varchar(255),
	sip_domain varchar(255),
	primary key (location_id)
);

create sequence location_seq;

insert into initialization_task (name) values ('migrate_locations');