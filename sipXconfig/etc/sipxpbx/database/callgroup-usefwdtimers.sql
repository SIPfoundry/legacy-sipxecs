alter table call_group add column use_fwd_timers boolean;

-- support older postgres (7.4)
alter table call_group alter column use_fwd_timers set default false;
update call_group set use_fwd_timers = false;

