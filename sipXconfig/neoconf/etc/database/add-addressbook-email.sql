alter table address_book_entry
  add column email_address character varying(255);
alter table address_book_entry
  add column alternate_email_address character varying(255);
insert into initialization_task (name) values ('mailbox_prefs_migration');
