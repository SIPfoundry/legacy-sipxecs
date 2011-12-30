insert into value_storage (value_storage_id) values (1);

insert into group_storage (resource, weight, name, group_id) values
 ('unittest', 1000, 'food', 1);
 
insert into setting_value (value_storage_id, value, path) values
 (1, 'macintosh', 'fruit/apple'),
 (1, 'snap pea', 'vegetable/pea');
