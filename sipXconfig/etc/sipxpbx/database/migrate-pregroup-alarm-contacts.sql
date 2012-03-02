create or replace function migrate_pregroup_alarm_contacts() returns void as $$
declare
  index_contact integer;
  contact record;
  default_group_id integer;
begin
  SELECT alarm_group_id INTO default_group_id FROM alarm_group WHERE name = 'default';

  index_contact := 0;

  for contact in select * from alarm_contacts loop
    insert into alarm_group_emailcontacts values(default_group_id, contact.address, index_contact);
	index_contact := index_contact + 1;
  end loop;
end;
$$ language plpgsql;

select migrate_pregroup_alarm_contacts();

drop table alarm_contacts;
