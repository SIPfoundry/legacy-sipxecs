-- fix the length of some columns
alter table address_book_entry drop column office_address_country;
alter table address_book_entry drop column office_address_state;

alter table address_book_entry add column office_address_country character varying(255);
alter table address_book_entry add column office_address_state character varying(255);

-- create sequence for address_book_entry table
create sequence abe_seq;