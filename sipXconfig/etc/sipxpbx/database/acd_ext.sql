-- add extension column to ACD table
alter table acd_line add column extension varchar(255);

-- add overflow queue id to queue table
alter table acd_queue add column overflow_queue_id int;

-- migrate data from previous version
insert into initialization_task (name) values ('acd_migrate_line_extensions');

insert into initialization_task (name) values ('acd_migrate_overflow_queues');
