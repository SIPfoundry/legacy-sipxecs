update users set pintoken = a.im_password from users u
  inner join address_book_entry a 
  on u.address_book_entry_id = a.address_book_entry_id
  where a.im_password is not NULL;

alter table address_book_entry drop column im_password;