create or replace function migrate_openacd_skill_groups() returns void as $$
declare
 skill_rec record;
begin
 for skill_rec in select * from openacd_skill
 loop
   if (select count(*) from openacd_skill_group where name = skill_rec.group_name) = 0
   then
     insert into openacd_skill_group(openacd_skill_group_id, name) values(nextval('openacd_skill_group_seq'), skill_rec.group_name);
     update openacd_skill set openacd_skill_group_id = (select currval('openacd_skill_group_seq'))
     where
      openacd_skill.openacd_skill_id = skill_rec.openacd_skill_id;
   else
     update openacd_skill set openacd_skill_group_id =
      (select openacd_skill_group_id from openacd_skill_group where name = skill_rec.group_name)
      where
      openacd_skill.openacd_skill_id = skill_rec.openacd_skill_id;
   end if;
 end loop;
end;
$$ language plpgsql;

select migrate_openacd_skill_groups();

alter table openacd_skill drop column group_name;

delete from initialization_task where name = 'skill_group_name_migrate_skill_group';
