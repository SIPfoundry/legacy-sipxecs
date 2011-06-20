-- modify gateway table to add sbc_device_id column
alter table gateway add sbc_device_id int4;

-- modify gateway table to add foreign key constraint to sbc_device table
alter table gateway add constraint fk_sbc_device_id foreign key (sbc_device_id)
	references sbc_device(sbc_device_id) match full;

insert into initialization_task (name) values ('sip_trunk_address_migrate_sbc_device');