-- careful - updates with joins are silently misleading in postgres. google it
update users u set pintoken = x.im_password from (select a.im_password, a.address_book_entry_id from address_book_entry a join users u2 on  u2.address_book_entry_id = a.address_book_entry_id and a.im_password is not NULL) x where u.address_book_entry_id = x.address_book_entry_id;

alter table address_book_entry drop column im_password;