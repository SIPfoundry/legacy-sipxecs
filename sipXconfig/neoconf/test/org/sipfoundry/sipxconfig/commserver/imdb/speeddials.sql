insert into users (user_name, user_id, pintoken, user_type) values
 ('user_a', 9991, '1234', 'C');
insert into users (user_name, user_id, pintoken, user_type) values
 ('user_b', 9992, '1234', 'C');
insert into users (user_name, user_id, pintoken, user_type) values
 ('user_c', 9993, '1234', 'C');
insert into users (user_name, user_id, pintoken, user_type) values
 ('user_d', 9994, '1234', 'C');

insert into value_storage (value_storage_id) values
 (1001);

insert into group_storage(group_id, resource, weight, name) values 
 (1001, 'user', 1001, 'group1');
 
insert into user_group (group_id, user_id) values
 (1001, 9991);
insert into user_group (group_id, user_id) values
 (1001, 9992);
 
insert into speeddial(speeddial_id, user_id) values (2000,9991);
insert into speeddial(speeddial_id, user_id) values (2002,9994);
insert into speeddial_group(speeddial_id, group_id) values (2001,1001);

insert into speeddial_button(position, speeddial_id, label, number, blf)
  values(0, 2000, 'alpha', '101', false);
insert into speeddial_button(position, speeddial_id, label, number, blf)
  values(1, 2000, 'beta', '102', true);
insert into speeddial_button(position, speeddial_id, label, number, blf)
  values(2, 2000, 'gamma', '104@sipfoundry.org', true);
  
insert into speeddial_group_button(position, speeddial_id, label, number, blf)
  values(0, 2001, 'alpha1', '202', false);
insert into speeddial_group_button(position, speeddial_id, label, number, blf)
  values(1, 2001, 'beta1', '404', true);
  
insert into speeddial_button(position, speeddial_id, label, number, blf)
  values(0, 2002, 'alpha', '101', true);