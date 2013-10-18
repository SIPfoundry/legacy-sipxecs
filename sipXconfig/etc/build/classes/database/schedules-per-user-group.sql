alter table schedule add group_id int4;
alter table schedule add schedule_type char(1);
alter table schedule add constraint fk_schedule_user_groups
  foreign key (group_id) references group_storage (group_id)
  on update no action on delete no action;
update schedule set schedule_type='S';
