create or replace function system_audit_cleanup() returns void as $$
declare
    default_setting varchar default 30;
    value_setting varchar;
    days_to_keep varchar;
begin
    select value into value_setting from setting_value where path = 'config-change-audit/keep-changes';
    days_to_keep := COALESCE(value_setting, default_setting);
    delete from config_change where age(date_time)>(days_to_keep || ' days')::interval;

    delete from user_ip_address where user_ip_address_id not in (select user_ip_address_id from config_change);
end;
$$ language plpgsql;
