alter table call_group add column voicemail_fallback bool default false;

-- support older postgres (7.4)
update call_group set voicemail_fallback = 'false';
alter table call_group alter column voicemail_fallback set not null; 
