insert into users (user_id, user_name, pintoken, user_type) values
 (1001, 'user1', 'x', 'C'),
 (1002, 'user2', 'x', 'C'),
 (1003, 'user3', 'x', 'C'),
 (1004, 'user4', 'x', 'C');
 
insert into paging_group (paging_group_id, page_group_number, description, enabled, sound, timeout) values
 (100, '111', 'Sales', true, 'TadaTada.wav', 60),
 (101, '112', 'Engineering', false, 'TadaTada.wav', 60),
 (102, '113', 'Support', true, 'fanfare.wav', 600);

insert into user_paging_group (paging_group_id, user_id) values
 (100, 1001),
 (100, 1002),
 (101, 1002),
 (102, 1001),
 (102, 1002),
 (102, 1003);
