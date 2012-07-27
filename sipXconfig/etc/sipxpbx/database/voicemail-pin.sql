update users set pintoken = a.im_password 
from users u
inner join 
 address_book_entry a
on
 u.address_book_entry_id = a.address_book_entry_id
where
 a.im_password is not NULL;

alter table users
  add column voicemail_pintoken varchar(255);