alter table phonebook_file_entry add phonebook_entry_type char(1);
alter table phonebook_file_entry add google_account_id varchar(255);
alter table phonebook_file_entry add internal_id varchar(255);
update phonebook_file_entry set phonebook_entry_type='F';
