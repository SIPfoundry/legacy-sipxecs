alter table backup_plan alter column backup_type type varchar(16);
update backup_plan set backup_type = 'ftp' where backup_type = 'F';
update backup_plan set backup_type = 'local' where backup_type != 'ftp';

alter table backup_plan add column def varchar(256);
-- not a perfect translation, but good enough
update backup_plan set def = def || ',vm.tgz' where voicemail = TRUE;
update backup_plan set def = def || ',admin.tgz' where configs = TRUE or def is NULL;
-- remove preceeding comma
update backup_plan set def = substr(def, 2);
alter table backup_plan alter column def set not NULL;

alter table backup_plan drop column email_address;
alter table backup_plan drop column voicemail;
alter table backup_plan drop column configs;
alter table backup_plan drop column ftp_configuration_id;

