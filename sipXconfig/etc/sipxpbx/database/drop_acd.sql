-- delete first works because no FKs from acd_* tables to value_storage
delete from setting_value where value_storage_id in (select value_storage_id from acd_agent);
delete from value_storage where value_storage_id in (select value_storage_id from acd_agent);
delete from setting_value where value_storage_id in (select value_storage_id from acd_line);
delete from value_storage where value_storage_id in (select value_storage_id from acd_line);
delete from setting_value where value_storage_id in (select value_storage_id from acd_queue);
delete from value_storage where value_storage_id in (select value_storage_id from acd_queue);
delete from setting_value where value_storage_id in (select value_storage_id from acd_server);
delete from value_storage where value_storage_id in (select value_storage_id from acd_server);
-- drop in order to avoid FKs
drop table acd_agent_queue cascade;
drop table acd_queue_agent cascade;
drop table acd_line_queue cascade;
drop table acd_agent cascade;
drop table acd_line cascade;
drop table acd_server cascade;
drop sequence acd_seq;
