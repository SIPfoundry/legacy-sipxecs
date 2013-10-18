alter table phonebook
  add column user_id int4;

alter table phonebook add constraint fk_phonebook_users foreign key (user_id) references users;
