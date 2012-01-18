-- supervisor1
--    group1
--      peon1
--      peon2
--    group3
--      peon1
--      peon5
--    group4
--    --empty-
--  supervisor2
--     group2
--      peon3
insert into users (user_name, user_id, pintoken, user_type) values
 ('peon1', 1001, '1234', 'C'),
 ('peon2', 1002, '1234', 'C'),
 ('peon3', 1003, '1234', 'C'),
 ('peon4', 1004, '1234', 'C'),
 ('peon5', 1005, '1234', 'C'),
 ('supervisor1', 2001, '1234', 'C'),
 ('supervisor2', 2002, '1234', 'C');

 insert into value_storage (value_storage_id) values
 (1001), (1002), (1003), (1004); 
 
insert into group_storage(group_id, resource, weight, name) values 
 (1001, 'user', 1001, 'Group1'),
 (1002, 'user', 1002, 'Group2'),
 (1003, 'user', 1003, 'Group3'),
 (1004, 'user', 1004, 'Group4');

insert into supervisor (group_id, user_id) values
-- supervisor1 => groups 1,3,4
 (1001, 2001),
 (1003, 2001),
 (1004, 2001),
 -- supervisor2 => groups 2
 (1002, 2002);

 insert into user_group (group_id, user_id) values
 -- group1 => peons 1,2
 (1001, 1001),
 (1001, 1002),
 -- group2 => peons 1,2
 (1002, 1003),
 -- group3 => peons 1,5
 (1003, 1001),
 (1003, 1005);
 
-- group4 => empty