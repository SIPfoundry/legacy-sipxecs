create table address_book_entry
(
  address_book_entry_id integer not null,
  cell_phone_number character varying(255),
  home_phone_number character varying(255),
  assistant_name character varying(255),
  assistant_phone_number character varying(255),
  fax_number character varying(255),
  location character varying(255),
  company_name character varying(255),
  home_address_street character varying(255),
  home_address_zip character varying(255),
  home_address_country character varying(255),
  home_address_state character varying(255),
  home_address_city character varying(255),
  office_address_street character varying(255),
  office_address_zip character varying(255),
  office_address_country character varying(255),
  office_address_state character varying(255),
  office_address_city character varying(255),
  job_title character varying(255),
  job_dept character varying(255),
  office_address_office_designation character varying(255),
  im_id character varying(255),
  alternate_im_id character varying(255),
  constraint address_book_entry_pkey primary key (address_book_entry_id)
);

alter table users
  add column address_book_entry_id integer;
alter table users
  add constraint fk_address_book_entry_users_id
  foreign key (address_book_entry_id)
  references address_book_entry;
