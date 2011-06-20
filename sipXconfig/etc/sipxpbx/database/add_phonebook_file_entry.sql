create table phonebook_file_entry (
  phonebook_file_entry_id integer not null,
  first_name character varying(255),
  last_name character varying(255),
  phone_number character varying(255),
  address_book_entry_id integer,
  phonebook_id integer not null,
  constraint phonebook_file_entry_pkey primary key (phonebook_file_entry_id)
);

create sequence phonebook_file_entry_seq;

alter table phonebook_file_entry
  add constraint phonebook_entry_phonebook foreign key (phonebook_id)
  references phonebook;
alter table phonebook_file_entry
  add constraint phonebook_entry_address_book_entry foreign key (address_book_entry_id)
  references address_book_entry;

insert into initialization_task (name) values ('phonebook_file_entry_task');
