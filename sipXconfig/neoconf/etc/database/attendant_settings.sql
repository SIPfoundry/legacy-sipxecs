alter table auto_attendant add column value_storage_id int4;
alter table auto_attendant add column group_id int4;

create table attendant_group (
   auto_attendant_id int4 not null,
   group_id int4 not null,
   primary key (auto_attendant_id, group_id)
);
