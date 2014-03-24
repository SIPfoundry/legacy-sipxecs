-- delete FKs from acd_* tables to value_storage
alter table acd_agent drop constraint "fkab9535086c899bce";
alter table acd_line drop constraint "fk816cf1916c899bce";
alter table acd_queue drop constraint "fkac7d0b146c899bce";
alter table acd_server drop constraint "fke5b27da06c899bce";

-- delete FKs from acd_* tables to users and location
alter table acd_agent drop constraint "fkab953508a53eb3e4";
alter table acd_server drop constraint "fk_location_id";

delete from setting_value where value_storage_id in (select value_storage_id from acd_agent);
delete from value_storage where value_storage_id in (select value_storage_id from acd_agent);
delete from setting_value where value_storage_id in (select value_storage_id from acd_line);
delete from value_storage where value_storage_id in (select value_storage_id from acd_line);
delete from setting_value where value_storage_id in (select value_storage_id from acd_queue);
delete from value_storage where value_storage_id in (select value_storage_id from acd_queue);
delete from setting_value where value_storage_id in (select value_storage_id from acd_server);
delete from value_storage where value_storage_id in (select value_storage_id from acd_server);

-- drop tables in order to avoid FKs
drop table acd_agent_queue cascade;
drop table acd_queue_agent cascade;
drop table acd_line_queue cascade;
drop table acd_agent cascade;
drop table acd_line cascade;
drop table acd_queue cascade;
drop table acd_server cascade;
drop sequence acd_seq;
