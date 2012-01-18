insert into users (user_name, user_id, pintoken, user_type) values
 ('user1000', 1000, '1234', 'C'),
 ('yellowthroat', 1001, '1234', 'C'),
 ('portaluser', 1002, '1234', 'C'),
 ('anotheruser', 1003, '1234', 'C');
 
insert into value_storage (value_storage_id) values
 (1000), (1001), (1002), (1003);
 
insert into group_storage (group_id, resource, weight, name) values
 (1000, 'user', 1000, '1000'),
 (1001, 'user', 1001, 'warblers'),
 (1003, 'user', 1003, '1003');
 
insert into user_group (group_id, user_id) values
 (1000, 1000),
 (1001, 1001),
 (1003, 1003);
 
insert into phonebook (name, phonebook_id, user_id) values
 ('test1000', 1000, null),
 ('test', 1001, null),
 ('privatePhonebook_1002', 1002, 1002),
 ('privatePhonebook_1003', 1003, 1003);

insert into phonebook_member (phonebook_id, group_id) values
 (1000, 1000), 
 (1001, 1001);

insert into phonebook_consumer (phonebook_id, group_id) values
 (1000, 1000),
 (1001, 1003);

insert into phonebook_file_entry (phonebook_file_entry_id, phonebook_id, phonebook_entry_type) values
 (2, 1002, 'F'),
 (3, 1003, 'F');

 insert into google_domain (domain_name, google_domain_id) values
 ('mydomain.com', 1);
