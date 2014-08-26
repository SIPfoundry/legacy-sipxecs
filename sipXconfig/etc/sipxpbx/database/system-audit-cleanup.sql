create or replace function system_audit_cleanup() returns void as $$
declare
    default_setting varchar default 30;
    value_setting varchar;
    days_to_keep varchar;
begin
    select value into value_setting from setting_value where path = 'config-change-audit/keep-changes';
    days_to_keep := COALESCE(value_setting, default_setting);
    delete from config_change where age(date_time)>(days_to_keep || ' days')::interval;
end;
$$ language plpgsql;
