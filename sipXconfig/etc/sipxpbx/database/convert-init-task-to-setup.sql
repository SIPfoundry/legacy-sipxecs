-- these tasks we only want to run once and they could only be there because a sql patch added
-- them after modifying the schema so we copy them into setup table as things to do. 
insert into setup select name from initialization_task where name in (
  'callgroup-password-init',
  'cleanup-dial-plans',
  'sip_trunk_address_migrate_sbc_device',
  'phonebook_file_entry_task',
  'phonebook_entries_update_task',
  'sbc_address_migrate_sbc_device'
);

drop table initialization_task;
