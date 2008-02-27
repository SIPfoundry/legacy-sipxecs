alter table call_group add column user_forward boolean;

-- support older postgres (7.4)
alter table call_group alter column user_forward set default true;
update call_group set user_forward = true;
alter table call_group alter column user_forward set not null;

