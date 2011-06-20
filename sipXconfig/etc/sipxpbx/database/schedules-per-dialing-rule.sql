alter table dialing_rule add schedule_id int4;
alter table dialing_rule add constraint fk_dialing_rule_schedule_id foreign key (schedule_id)
	references schedule(schedule_id) match full;