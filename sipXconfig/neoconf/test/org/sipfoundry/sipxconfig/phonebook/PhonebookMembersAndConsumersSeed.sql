
insert into users (user_name, user_id, pintoken, address_book_entry_id, user_type) values
-- warblers
 ('yellowthroat',	1001, '1234', null, 'C'),
 ('canadian', 		1002, '1234', null, 'C'),
-- sparrows 
 ('chirping', 		1003, '1234', null, 'C'),
 ('song', 			1004, '1234', null, 'C'),
-- ducks 
 ('mallard', 		1005, '1234', null, 'C'),
 ('pintail', 		1006, '1234', null, 'C');
 
insert into value_storage (value_storage_id) values
 (1001), (1002), (1003);
 
insert into group_storage (group_id, resource, weight, name) values
 (1001, 'user', 1001, 'warblers'),
 (1002, 'user', 1002, 'sparrows'),
 (1003, 'user', 1003, 'ducks');

-- User Groups 
--   warblers: yellowthroat, canadian
--   sparrows: chirping, song
--   ducks: mallard, pintail
insert into user_group (group_id, user_id) values
 (1001, 1001),  
 (1001, 1002),  
 (1002, 1003),
 (1002, 1004),
 (1003, 1005),
 (1003, 1006);
 
-- Phonebooks
--   test1 : Members: warblers, Consumers: warblers
--   test3 : Members: ducks		Consumers: warblers
-- Summary
--   warblers should see all ducks and other warblers 
insert into phonebook (name, phonebook_id) values
 ('test1', 1001),
 ('test2', 1002),
 ('test3', 1003);

insert into phonebook_member (phonebook_id, group_id) values
 (1001, 1001), 
 (1003, 1003);

insert into phonebook_consumer (phonebook_id, group_id) values
 (1001, 1001),
 (1003, 1001);

insert into phonebook (name, phonebook_id, user_id, show_on_phone) values
 ('private', 1004, 1002, false);

insert into phonebook_file_entry (phonebook_file_entry_id, first_name, phone_number, phonebook_id, phonebook_entry_type) values
 (101, 'John', '10020', 1004, 'U');

insert into google_domain (domain_name, google_domain_id) values
 ('mydomain.com', 1);
