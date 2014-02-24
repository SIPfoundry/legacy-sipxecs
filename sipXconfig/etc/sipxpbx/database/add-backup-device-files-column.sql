create or replace function set_include_device_files() returns void as $$
begin
if (select count(*) from setting_value where path='backup/device') > 0
then 
  update backup_plan set include_device_files = false;
else
  update backup_plan set include_device_files = true;
end if;
end;
$$ language plpgsql;

alter table backup_plan add column include_device_files boolean;
select set_include_device_files();