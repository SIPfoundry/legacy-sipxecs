alter table call_group add column user_forward boolean;
alter table call_group alter column user_forward set default true;

-- support older postgres (7.4)
update call_group set user_forward = true;
alter table call_group alter column user_forward set not null;

