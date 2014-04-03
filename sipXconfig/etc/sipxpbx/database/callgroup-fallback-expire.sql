alter table call_group add column fallback_expire integer not null default 30;
update call_group set fallback_expire = 30;