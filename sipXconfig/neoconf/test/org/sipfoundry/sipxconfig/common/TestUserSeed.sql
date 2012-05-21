insert into address_book_entry (address_book_entry_id) values (1);

insert into users (user_id, user_name, first_name, last_name, sip_password, pintoken, address_book_entry_id, user_type, voicemail_pintoken) values
  (1000, 'testuser', 'Test', 'User', '1234', '1234', 1, 'C', '555'),
  (2001, '~~tp~internal~', '', '', '1234', '', null, 'I', '556');
  
insert into user_alias (user_id, alias) values 
  (1000, '123');  
