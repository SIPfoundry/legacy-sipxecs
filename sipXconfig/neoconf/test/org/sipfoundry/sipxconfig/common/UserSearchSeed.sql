insert into value_storage (value_storage_id) values 
  (1001), (1002);
  
insert into address (address_id, street) values
  (1, '1 Drive');

insert into address_book_entry (address_book_entry_id, office_address_id, im_id) values
  (1, 1, 'jagr');

insert into address_book_entry (address_book_entry_id) values
  (2), (3), (4), (5), (6), (7), (8), (9), (10);

insert into group_storage(group_id, name, resource, description, weight) values
  (1001, 'user group 1', 'user', 'user group 1', 0),
  (1002, 'user group 2', 'user', 'user group 2', 0);

insert into users (first_name, last_name, user_name, pintoken, user_id, address_book_entry_id, user_type) values
  ('', '', 'userseed1', 'x', 1001, 1, 'C'),
  ('', '', 'userseed2', 'x', 1002, 2, 'C'),
  ('', '', 'userseed3', 'x', 1003, 3, 'C'),
  ('user4', '', 'userseed4', 'x', 1004, 4, 'C'),
  ('', 'seed5', 'userseed5', 'x', 1005, 5, 'C'),
  ('', '', 'userseed6', 'x', 1006, 6, 'C'),
  ('', '', 'usersee-nofind', 'x', 1007, 7, 'C'),
  ('', '', 'canfind-userseed', 'x', 1008, 8, 'C'),
  ('', '', 'u s e r s e e d', 'x', 1009, 9, 'C'),
  ('', '', 'userwithnoaliases', 'x', 1010, 10, 'C'),  
  ('', '', '~~tp~tlspeer', 'x', 2007, null, 'I');
  
insert into user_group (user_id, group_id) values
  (1001, 1001),
  (1008, 1001),
  (1001, 1002),
  (1004, 1002),
  (1010, 1002);

insert into user_alias (user_id, alias) values
  (1001, '1'),
  (1002, '2'),
  (1002, 'two'),
  (1003, '3'),
  (1004, '4'),
  (1005, '5'),
  (1006, '6'),
  (1007, '7'),
  (1008, '8'),
  (1008, '9');
