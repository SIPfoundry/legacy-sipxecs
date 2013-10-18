-- hopefully you've already used these tables to convert the data first (example: meetme_location.sql)
drop table location_specific_service;
drop table sipx_service;
drop sequence sipx_service_seq;
drop table location_bundle;
drop sequence location_specific_service_seq;

-- not bothering to migrate ATM, user can recreate. it's only enable/disable everyone setting
delete from setting_value where value_storage_id in (select value_storage_id from general_phonebook_settings);
delete from value_storage where value_storage_id in (select value_storage_id from general_phonebook_settings);
drop table general_phonebook_settings;
drop sequence general_phonebook_settings_seq;
