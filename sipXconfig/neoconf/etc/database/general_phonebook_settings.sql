create table general_phonebook_settings (
   general_phonebook_settings_id int4 not null,
   value_storage_id int4,
   primary key (general_phonebook_settings_id)
);

create sequence general_phonebook_settings_seq;