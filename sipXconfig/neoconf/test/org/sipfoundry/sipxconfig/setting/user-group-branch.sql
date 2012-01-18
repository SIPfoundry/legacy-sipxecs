insert into branch (branch_id, name, description) values
  (1000, 'branch1',''),
  (1001, 'branch2',''),
  (1002, 'branch3',''); 

insert into users (user_name, user_id, branch_id, user_type) values
  ('user1', 1000, 1000, 'C'),
  ('user2', 1001, null, 'C'),
  ('user3', 1002, 1001, 'C'),
  ('user4', 1003, 1001, 'C'),
  ('user5', 1004, null, 'C'),
  ('user6', 1005, 1002, 'C');

insert into value_storage (value_storage_id) values
 (1000), (1001), (1002), (1003);

insert into group_storage (group_id, resource, weight, name, branch_id) values
 (1000, 'user', 1000, 'group1', 1000),
 (1001, 'user', 1001, 'group2', 1001),
 (1002, 'user', 1002, 'group3', 1002),
 (1003, 'user', 1003, 'group4', null);
 
insert into user_group (user_id, group_id) values
 (1000, 1000),
 (1001, 1000),
 (1004, 1001);
