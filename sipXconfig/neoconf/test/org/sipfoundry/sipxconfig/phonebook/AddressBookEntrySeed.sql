insert into address (address_id, street) values
 (2, 'branchAddress'),
 (1, null);
 
insert into branch (branch_id, address_id) values
 (1, 2);
 
insert into address_book_entry (address_book_entry_id, office_address_id) values
 (1, 1);
 
insert into users (first_name, last_name, user_name, pintoken, user_id, address_book_entry_id, user_type, branch_id) values
 ('', '', 'userseed1' , 'x', 1001, 1, 'C', 1);
