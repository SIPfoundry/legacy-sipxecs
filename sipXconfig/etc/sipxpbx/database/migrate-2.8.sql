--  Dont use apostrophes in comments inside functions
--  Double all single quotes in all function bodies.
--  Dont use variable names that match tables or column names when setting 
--    variable (see my_* usages)
--  primitive logging by raising notices.
--  some functions preserve primary keys from PDS, others do not.  depends on
--    how easy it is to adjust
--    and if destination table has to merge values from multiple tables in 
--    old schema.  if primary keys
--    are preserved, sequences must be updated manually
--  raise notice with prefix string "DATA LOSS:" if records could not be 
--    translated as an audit trail for admins to handle manually
--  postgres 8 more picky and error much more informative
--  when developing and wishing to run w/o installing, I recommend this line:
/*
psql -a --set ON_ERROR_STOP=true -U postgres SIPXCONFIG_TEST < migrate-2.8.sql
*/

-- U S E R   G R O U P S
create or replace function migrate_user_groups() returns integer as '
declare
  usrgrp record;
  next_id int;
begin

  -- cannot insert phone group id directly into new database, ids would conflict with
  -- user groups as the share same tables
  create temporary table user_group_migration (pds_group_id int not null, group_id int not null);

  -- todo user permissions
  raise notice ''DATA LOSS: User group permissions not migrated'';

  for usrgrp in select * from dblink(''select id, name from user_groups'') 
      as (id int, name text) loop

    raise notice ''importing user group %...'', usrgrp.name;

    next_id := nextval(''storage_seq'');

    insert into user_group_migration (pds_group_id, group_id) values (usrgrp.id, next_id);
 
    insert into value_storage (value_storage_id) values (next_id);

    insert into group_storage (group_id, name, weight, resource)
        values (next_id, usrgrp.name, nextval(''group_weight_seq''), ''user'');
    
  end loop; 

  return 1;
end;
' language plpgsql;


create or replace function migrate_group_weights() returns integer as '
declare
  my_group record;
  my_weight int := 0;
  next_id int;
begin

  -- default group has the most members and the lowest weight. 
  -- sort by member count descending and assign next weight seq.
  for my_group in 
    select g.group_id as id from user_group ug
      left join group_storage g on ug.group_id = g.group_id
      group by g.group_id order by count(*) desc loop

    update group_storage set weight=my_weight where group_id = my_group.id;
    my_weight := my_weight + 1;

  end loop;

  my_weight := 0;
  for my_group in 
    select g.group_id as id from phone_group pg
      left join group_storage g on pg.group_id = g.group_id
      group by g.group_id order by count(*) desc loop

    update group_storage set weight=my_weight where group_id = my_group.id;
    my_weight := my_weight + 1;

  end loop;

  next_id := max(weight) + 1 from group_storage;
  perform setval(''group_weight_seq'', next_id);

  return 1;
end;
' language plpgsql;


-- U S E R S
create or replace function migrate_users() returns integer as '
declare
  usr record;
  next_id int;
  my_group_id int;
begin

  -- todo report user permissions that cannot be handled
  raise notice ''DATA LOSS: User permissions not migrated'';

  -- avoid SDS and superadmin users
  for usr in select * from dblink(''select id, first_name, password, 
      last_name, display_id, extension, ug_id 
     from users where display_id != ''''SDS'''''') 
       as (id int, first_name text, password text, last_name text, 
       display_id text, extension text, ug_id int) loop

    raise notice ''importing user %...'', usr.display_id;

    insert into users (user_id, first_name, pintoken, last_name, user_name)
      values (usr.id, usr.first_name, usr.password, usr.last_name, 
        usr.display_id);

    -- user group
    if usr.ug_id is not null then
      select into my_group_id group_id from user_group_migration where pds_group_id = usr.ug_id;
      insert into user_group (user_id, group_id) values (usr.id, my_group_id);
    end if;

  end loop; 

  next_id := max(user_id) + 1 from users;
  perform setval(''user_seq'', next_id);

  return 1;
end;
' language plpgsql;


create or replace function migrate_sip_passwords() returns integer as '
declare
  my_sip_password record;
begin

  for my_sip_password in select * from dblink(''select u.usrs_id, 
      substring(substring(cs.content from ''''line1_password>.*</line1_password'''') 
      from ''''[0-9]{2,16}'''') 
    from 
      config_sets cs, user_cs_assoc u 
    where 
      u.cs_id = cs.id'') as (user_id int, password text)
  loop
    update users set sip_password = my_sip_password.password where user_id = my_sip_password.user_id;
  end loop;

  

  return 1;
end;
' language plpgsql;


create or replace function my_template_function() returns integer as '
begin
  return 1;
end;
' language plpgsql;

-- U S E R   A L I A S E S
create or replace function migrate_aliases() returns integer as '
declare
  duplicate_alias_query text := ''select a1.alias, a1.user_id from 
       aliases a1, aliases a2 
    where a1.alias = a2.alias and a1.user_id != a2.user_id'';
  duplicate_alias record;
  user_name_alias_conflict record;
begin

  -- pds primary key on user and alias, so aliases are not 
  -- gauronteed to be unique. spit out bad records for audit trail
  for duplicate_alias in select * from dblink(duplicate_alias_query) as 
        (alias text, user_id int) loop
    raise notice ''DATA LOSS: multiple users for single alias % (user_id=%)'',
        duplicate_alias.alias, duplicate_alias.user_id;
  end loop;

  insert into user_alias (alias, user_id) select * from
    dblink(''select a.alias, a.user_id from aliases a except '' ||
        duplicate_alias_query) 
    as (alias text, user_id int);
 
  -- remove aliases that match existing user_names
  for user_name_alias_conflict in 
    select au.user_name as user_name, a.alias as alias
    from user_alias a, users u, users au 
    where a.alias = u.user_name and a.user_id = au.user_id
  loop
    raise notice ''DATA LOSS: user % has alias % matching another user'',
      user_name_alias_conflict.user_name, user_name_alias_conflict.alias;
    delete from user_alias where alias = user_name_alias_conflict.alias;
  end loop;

  return 1;
end;
' language plpgsql;

-- U S E R  E X T E N S I O N S
create or replace function migrate_extensions() returns integer as '
declare
  user_extension record;
  alias_conflict record;
begin

  for user_extension in select * from dblink(
      ''select id, extension from users where extension is not null'')
      as (user_id int, extension text) loop

    select into alias_conflict * from user_alias 
        where alias = user_extension.extension;
    if found then
      raise notice ''DATA LOSS: extension conflicts with alias % '',
          user_extension.extension;
    else
      insert into user_alias (user_id, alias) 
          values (user_extension.user_id, user_extension.extension);
    end if;

  end loop;

  return 1;
end;
' language plpgsql;



-- U S E R  G R O U P  H I E R A R C H Y
create or replace function migrate_user_group_tree() returns integer as '
declare
  not_supported_notice record;
begin

  -- level 1
  perform insert_users_into_groups(''
  select 
    u.id, g1.id from users u, user_groups g, user_groups g1
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id'');

  -- level 2
  perform insert_users_into_groups(''
  select 
    u.id, g2.id from users u, user_groups g, user_groups g1, user_groups g2
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id'');

  -- level 3
  perform insert_users_into_groups(''
  select 
    u.id, g3.id from users u, user_groups g, user_groups g1, user_groups g2, user_groups g3
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id and 
    g2.parent_ug_id = g3.id'');

  -- level 4 and above not supported
  select into not_supported_notice * from dblink(''
  select 
    u.id, g4.id from users u, user_groups g, user_groups g1, user_groups g2, user_groups g3, user_groups g4
  where 
    u.ug_id = g.id and g.parent_ug_id = g1.id and g1.parent_ug_id = g2.id and 
    g2.parent_ug_id = g3.id and g3.parent_ug_id = g4.id'') 
  as (user_id int, group_id int);
  if found then
    raise exception ''DATA LOSS: cannot support user group levels 4 parents or deeper'';
  end if;

  return 1;
end;
' language plpgsql;

-- help function for user tree function
create or replace function insert_users_into_groups(varchar) returns integer as '
declare
  user_select alias for $1;
  grp record;
  user_group_id int;
begin
  for grp in select * from dblink(user_select) as (user_id int, group_id int) loop
    select into user_group_id group_id from user_group_migration where pds_group_id = grp.group_id;
    insert into user_group (user_id, group_id) values (grp.user_id, user_group_id);
  end loop;

  return 1;
end;
' language plpgsql;

-- P H O N E  G R O U P S
create or replace function migrate_phone_groups() returns integer as '
declare
  phonegrp record;
  next_id int;
begin

  -- cannot insert phone group id directly into new database, ids would conflict with
  -- user groups as the share same tables
  create temporary table phone_group_migration (pds_group_id int not null, group_id int not null);

  for phonegrp in select * from dblink(''select id, name from phone_groups'') as (id int, name text) loop

    next_id := nextval(''storage_seq'');

    insert into phone_group_migration (pds_group_id, group_id) values (phonegrp.id, next_id);

    insert into value_storage (value_storage_id) values (next_id);

    insert into group_storage (group_id, name, weight, resource)
        values (next_id, phonegrp.name, nextval(''group_weight_seq''), ''phone'');

  end loop; 

  return 1;
end;
' language plpgsql;

-- P H O N E   G R O U P  H I E R A R C H Y
create or replace function migrate_phone_group_tree() returns integer as '
declare
  not_supported_notice record;
begin

  -- level 1
  perform insert_phones_into_groups(''
  select 
    p.id, g1.id from logical_phones p, phone_groups g, phone_groups g1
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id'');

  -- level 2
  perform insert_phones_into_groups(''
  select 
    p.id, g2.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id'');

  -- level 3
  perform insert_phones_into_groups(''
  select 
    p.id, g3.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2, phone_groups g3
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id and
    g2.parent_pg_id = g3.id'');

  -- level 4 and above not supported
  select into not_supported_notice * from dblink(''
  select 
    p.id, g4.id from logical_phones p, phone_groups g, phone_groups g1, phone_groups g2, phone_groups g3,
    phone_groups g4
  where 
    p.pg_id = g.id and g.parent_pg_id = g1.id and g1.parent_pg_id = g2.id and
    g2.parent_pg_id = g3.id and g3.parent_pg_id = g4.id'') 
  as (phone_id int, group_id int);
  if found then
    raise exception ''DATA LOSS: cannot support phone group levels 4 parents or deeper'';
  end if;

  return 1;
end;
' language plpgsql;

-- helper function to phone group tree function
create or replace function insert_phones_into_groups(varchar) returns integer as '
declare
  phone_select alias for $1;
  grp record;
  phone_group_id int;
begin

  for grp in select * from dblink(phone_select) as (phone_id int, group_id int) loop
    select into phone_group_id group_id from phone_group_migration where pds_group_id = grp.group_id;
    insert into phone_group (phone_id, group_id) values (grp.phone_id, phone_group_id);
  end loop;

  return 1;
end;
' language plpgsql;


-- N O N   P O L Y C O M   P H O N E S
create or replace function migrate_non_polycom_phones() returns integer as '
declare
  my_phone record;
  my_group_id int;
  my_phone_type record;
  next_id int;
begin

  -- translation table for phone types
  create temporary table phone_type_migration ( pds_model text, bean_id text, model_id text);
  insert into phone_type_migration values (''7960'', ''ciscoIp'', ''7960'');
  insert into phone_type_migration values (''7940'', ''ciscoIp'', ''7940'');
  insert into phone_type_migration values (''xpressa_strongarm_vxworks'', ''unmanagedPhone'', null);
  insert into phone_type_migration values (''ixpressa_x86_win32'', ''unmanagedPhone'', null);

  for my_phone in select * from 
      dblink(''select p.id, p.serial_number, p.pg_id, p.usrs_id, pt.model, p.description
      from logical_phones p, phone_types pt where p.pt_id = pt.id'') 
      as (id int, serial_number text, pg_id int, usrs_id int, pt_model text, description text) loop

    select into my_phone_type * from phone_type_migration where pds_model = my_phone.pt_model; 

    raise notice ''importing phone %, %...'', my_phone.serial_number, my_phone_type.bean_id;

    insert into phone (phone_id, serial_number, bean_id, model_id, description) 
      values (my_phone.id, my_phone.serial_number, my_phone_type.bean_id, my_phone_type.model_id,
              my_phone.description);

    select into my_group_id group_id from phone_group_migration where pds_group_id = my_phone.pg_id;

    insert into phone_group (phone_id, group_id) values (my_phone.id, my_group_id);

    if my_phone.usrs_id is not null 
    then
      insert into line (line_id, phone_id, position, user_id) 
        values (nextval(''line_seq''), my_phone.id, 0, my_phone.usrs_id);
    end if;


  end loop; 

  -- todo, phone settings, only cisco? very difficult
  raise notice ''DATA LOSS: Cisco and (i)xpressa settings not migrated'';

  -- update value_storage_seq  
  next_id := max(phone_id) + 1 from phone;
  perform setval(''phone_seq'', next_id);

  return 1;
end;
' language plpgsql;


-- P O L Y C O M   P H O N E S 
create or replace function migrate_polycom_phones() returns integer as '
declare
  my_phone record;
  my_line record;
  next_id int;
  default_group_id int;
begin

  select into default_group_id group_id from group_storage 
      where name = ''default'' and resource = ''phone'';

  for my_phone in select * from dblink(
    ''select p.phone_id, p.serial_number, p.name, p.factory_id, 
        p.storage_id, p.folder_id from phone p'') 
      as (id int, serial_number text, name text, factory_id text, 
        storage_id int, folder_id int) 
  loop
    next_id := nextval(''phone_seq'');
    insert into phone (phone_id, description, serial_number, value_storage_id, bean_id, model_id) 
      values (next_id, my_phone.name, my_phone.serial_number, my_phone.storage_id, ''polycom'',
              substring(my_phone.factory_id from ''\\\\d*$''));

    for my_line in select * from dblink(''select position, storage_id, user_id from 
        line where phone_id ='' || my_phone.id) as (position int, storage_id int, user_id int) loop
    
        insert into line (line_id, phone_id, position, user_id, value_storage_id) 
          values (nextval(''line_seq''), next_id, my_line.position, my_line.user_id, my_line.storage_id);
         
    end loop;
    
  end loop; 

  -- all polycom phones in default group implicitly 
  insert into phone_group 
    select p.phone_id, default_group_id from phone p where bean_id = ''polycom'';

  -- model 3000 is actually a H323 phone valid model is 4000
  update phone set model_id = ''4000'' where model_id = ''3000'' and bean_id = ''polycom'';

  return 1;
end;
' language plpgsql;

-- S E T T I N G S
create or replace function migrate_settings() returns integer as '
declare
  next_id int;
begin

 -- straight migration 

  insert into value_storage select * from 
   dblink(''select storage_id from storage'') as (value_storage_id int);

  insert into setting_value (value_storage_id, path, value) select * from
    dblink(''select storage_id, path, value from setting'') 
    as (storage_id int, path text, value text);

  -- update value_storage_seq  
  next_id := max(value_storage_id) + 1 from value_storage;
  perform setval(''storage_seq'', next_id);
  
  return 1;
end;
' language plpgsql;

-- F O L D E R  V A L U E S
create or replace function migrate_folder_values() returns integer as '
declare
  default_group_id int;
begin

  select into default_group_id group_id from group_storage 
      where name = ''default'' and resource = ''phone'';

  insert into setting_value (value_storage_id, path, value) 
    select default_group_id, * from
      dblink(''select path, value from folder_setting'') 
      as (path text, value text);

  return 1;
end;
' language plpgsql;

-- E X T E N S I O N  P O O L S
create or replace function migrate_extension_pools() returns integer as '
begin
  raise notice ''DATA LOSS: Extension pools not migrated'';
  return 1;
end;
' language plpgsql;

-- D I A L I N G  P L A N S
create or replace function migrate_dialing_plans() returns integer as '
declare
  next_id int;
  user_sensitive_routing bool;
begin
  -- GATEWAY
  insert into gateway
     (gateway_id, name, address, description, bean_id)
    select *, ''gwGeneric'' from 
    dblink(''select gateway_id, name, address, description
       from gateway'') 
       as (id int, name text, address text, description text);

  next_id := max(gateway_id) + 1 from gateway;
  perform setval(''gateway_seq'', next_id);

  -- DIAL PLAN
  insert into dial_plan select * from 
    dblink(''select * from dial_plan'') as (id int);

  next_id := max(dial_plan_id) + 1 from dial_plan;
  perform setval(''dial_plan_seq'', next_id);

  -- DIALING RULES
  insert into dialing_rule
     (dialing_rule_id, name, description, enabled, position, dial_plan_id)
    select * from 
    dblink(''select dialing_rule_id, name, description, enabled, position, dial_plan_id
       from dialing_rule'') 
       as (id int, name text, description text, enabled bool, position int, dial_plan_id int);

  next_id := max(dialing_rule_id) + 1 from dialing_rule;
  perform setval(''dialing_rule_seq'', next_id);

  insert into custom_dialing_rule 
     (custom_dialing_rule_id, call_pattern_digits, call_pattern_prefix)
    select * from 
     dblink(''select custom_dialing_rule_id, digits, prefix 
       from custom_dialing_rule'') 
       as (id int, digit text, prefix text);

  insert into custom_dialing_rule_permission
     (custom_dialing_rule_id, permission)
    select * from 
     dblink(''select custom_dialing_rule_id, permission
       from custom_dialing_rule_permission'') 
       as (id int, permission text);

  insert into dial_pattern
     (custom_dialing_rule_id, element_prefix, element_digits, index)
    select * from 
    dblink(''select custom_dialing_rule_id, prefix, digits, index
       from dial_pattern'') 
       as (id int, prefix text, digits int, index int);

  insert into dialing_rule_gateway
     (dialing_rule_id, gateway_id, index)
    select * from 
    dblink(''select dialing_rule_id, gateway_id, index
       from dialing_rule_gateway'') 
       as (id int, gateway_id int, index int);

  insert into emergency_dialing_rule
     (emergency_dialing_rule_id, optional_prefix, emergency_number, use_media_server)
    select * from 
    dblink(''select emergency_dialing_rule_id, optionalprefix, emergencynumber, usemediaserver
       from emergency_dialing_rule'') 
       as (id int, prefix text, number text, usemediaserver bool);
       
  -- warn about data loss if user forwarding was used
  select into user_sensitive_routing use_media_server from emergency_dialing_rule where use_media_server is true;
  if found then
  	raise notice ''DATA LOSS: User sensitive emergency routing settings not migrated'';
  end if;

  -- operator initialization task will trigger associate to all internal dialing rules created here
  insert into internal_dialing_rule
     (internal_dialing_rule_id, local_extension_len, voice_mail, voice_mail_prefix)
    select * from 
    dblink(''select internal_dialing_rule_id, localextensionlen, voicemail, voicemailprefix
       from internal_dialing_rule'') 
       as (id int, xlen int, vm text, vmprefix text);

  insert into international_dialing_rule
     (international_dialing_rule_id, international_prefix)
    select * from 
    dblink(''select international_dialing_rule_id, internationalprefix
       from international_dialing_rule'') 
       as (id int, prefix text);

  insert into local_dialing_rule
     (local_dialing_rule_id, external_len, pstn_prefix)
    select * from 
    dblink(''select local_dialing_rule_id, externallen, pstnprefix
       from local_dialing_rule'') 
       as (id int, len int, prefix text);

  insert into long_distance_dialing_rule
     (international_dialing_rule_id, area_codes, external_len, long_distance_prefix, 
       permission, pstn_prefix)
    select * from 
    dblink(''select international_dialing_rule_id, areacodes, externallen, longdistanceprefix,
         permission, pstnprefix
       from long_distance_dialing_rule'') 
       as (id int, areacodes text, len int, prefix text, 
         permission text, pstn_prefix text);

  insert into ring
     (ring_id, number, position, expiration, ring_type, user_id)
    select * from 
    dblink(''select ring_id, number, position, expiration, ring_type, user_id
       from ring'') 
       as (id int, number text, position int, expiration int, ring_type text, user_id int);

  -- data has already gone thru initialization, this would
  -- clobber all dialplans post migration
  delete from initialization_task where name = ''dial-plans'';

  next_id := max(ring_id) + 1 from ring;
  perform setval(''ring_seq'', next_id);

  return 1;
end;
' language plpgsql;




-- ********** END PL/pgSQL **************


-- ********** BEGIN SQL ****************

load 'dblink';

select dblink_connect('dbname=PDS');

-- delete bottom up
delete from emergency_dialing_rule;
delete from internal_dialing_rule;
delete from international_dialing_rule;
delete from local_dialing_rule;
delete from long_distance_dialing_rule;
delete from ring;
delete from custom_dialing_rule;
delete from custom_dialing_rule_permission;
delete from dial_pattern;
delete from gateway;
delete from dialing_rule;
delete from dial_plan;

delete from line;
delete from phone_group;
delete from phone;
delete from user_group;
delete from user_alias;
delete from users;

delete from group_storage;
delete from setting_value;
delete from value_storage;

-- migrate top down
select migrate_settings();
select migrate_user_groups();
select migrate_users();
select migrate_sip_passwords();
select migrate_aliases();
select migrate_extensions();
select migrate_user_group_tree();

select migrate_phone_groups();
select migrate_folder_values();
select migrate_non_polycom_phones();
select migrate_polycom_phones();
select migrate_phone_group_tree();

select migrate_group_weights();
select migrate_dialing_plans();
select migrate_extension_pools();

-- this will give superadmin user correct permissions on system startup
insert into initialization_task (name) values ('admin-group-and-user');

-- trigger replication after data migration
insert into initialization_task (name) values ('replicate');
