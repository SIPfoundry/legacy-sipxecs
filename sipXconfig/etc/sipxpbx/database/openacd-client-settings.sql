alter table openacd_client add column value_storage_id integer;
alter table openacd_client add constraint fk_openacd_client_value_storage
	foreign key (value_storage_id) references value_storage;
