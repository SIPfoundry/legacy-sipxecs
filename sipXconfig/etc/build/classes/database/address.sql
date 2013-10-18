create table address
(
  address_id integer not null,
  street character varying(255),
  zip character varying(255),
  country character varying(255),
  state character varying(255),
  city character varying(255),
  office_designation character varying(255),
  constraint address_pkey primary key (address_id)
);

alter table address_book_entry drop column home_address_street;
alter table address_book_entry drop column home_address_zip;
alter table address_book_entry drop column  home_address_country;
alter table address_book_entry drop column  home_address_state;
alter table address_book_entry drop column  home_address_city;
alter table address_book_entry drop column  office_address_street;
alter table address_book_entry drop column  office_address_zip;
alter table address_book_entry drop column  office_address_country;
alter table address_book_entry drop column  office_address_state;
alter table address_book_entry drop column  office_address_city;
alter table address_book_entry drop column  office_address_office_designation;

alter table address_book_entry add column home_address_id integer;
alter table address_book_entry add column office_address_id integer;
alter table address_book_entry add column branch_address_id integer;
alter table address_book_entry add constraint fk_home_address_id foreign key (home_address_id)
references address (address_id) match simple on update no action on delete no action;
alter table address_book_entry add constraint fk_office_address_id foreign key (office_address_id)
references address (address_id) match simple on update no action on delete set null;
alter table address_book_entry add constraint fk_branch_address_id foreign key (branch_address_id)
references address (address_id) match simple on update no action on delete set null;

create sequence addr_seq;

alter table branch drop column  address_street;
alter table branch drop column  address_zip;
alter table branch drop column  address_country;
alter table branch drop column  address_state;
alter table branch drop column  address_city;
alter table branch drop column  office_designation;

alter table branch add column address_id integer;
alter table branch add constraint fk_address_id foreign key (address_id)
references address (address_id) match simple on update no action on delete no action;
