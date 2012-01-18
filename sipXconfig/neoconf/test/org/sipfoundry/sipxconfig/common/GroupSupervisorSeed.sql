insert into users (user_name, user_id, pintoken, user_type) values
 ('user1', 1001, '1234', 'C');
 
insert into value_storage (value_storage_id) values
 (1001);
 
insert into group_storage(group_id, resource, weight, name) values 
 (1001, 'user', 1001, 'SeedUserGroup1');
 
insert into supervisor (group_id, user_id) values
 (1001, 1001);
