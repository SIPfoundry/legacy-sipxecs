insert into phone (serial_number, phone_id, description, bean_id, model_id) values
  ('000000000001', 1001, 'unittest-sample phone1', 'testPhone', 'testPhoneModel'),
  ('000000000002', 1002, 'unittest-sample phone2', 'testPhone', 'testPhoneModel'),
  ('000000000003', 1003, 'unittest-sample phone3', 'testPhone', 'testPhoneModel');

insert into value_storage (value_storage_id) values
  (1001),
  (1002);

insert into group_storage (group_id, resource, weight, name) values
  (1001, 'phone', 1001, 'SeedPhoneGroup1'),
  (1002, 'phone', 1002, 'SeedPhoneGroup2');

insert into phone_group (group_id, phone_id) values
  (1001, 1001),
  (1001, 1002),
  (1002, 1002);
