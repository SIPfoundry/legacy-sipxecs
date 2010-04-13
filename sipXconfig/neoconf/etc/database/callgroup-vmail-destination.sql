alter table call_group add column voicemail_fallback boolean;

-- support older postgres (7.4)
alter table call_group alter column voicemail_fallback set default false;
update call_group set voicemail_fallback = false;
alter table call_group alter column voicemail_fallback set not null;

-- if falback destition was not set enable voicemail_fallback to preserve the bahavior after upgrade
update call_group set voicemail_fallback = true where fallback_destination is null or fallback_destination = '' ;
