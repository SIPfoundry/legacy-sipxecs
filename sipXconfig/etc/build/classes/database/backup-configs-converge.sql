update backup_plan set configs = configs or dbase;
alter table backup_plan drop column dbase;

