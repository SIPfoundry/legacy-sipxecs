create or replace function migrate_pregroup_alarm_contacts() returns void as '\n
declare\n
  index_contact integer;\n
  contact record;\n
  default_group_id integer;\n
begin\n
  SELECT alarm_group_id INTO default_group_id FROM alarm_group WHERE name = ''default'';\n
\n
  index_contact := 0;\n
\n
  for contact in select * from alarm_contacts loop\n
    insert into alarm_group_emailcontacts values(default_group_id, contact.address, index_contact);\n
	index_contact := index_contact + 1;\n
  end loop;\n
end;\n
' language plpgsql;

select migrate_pregroup_alarm_contacts();

drop table alarm_contacts;
