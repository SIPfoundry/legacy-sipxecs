insert into sbc_device (sbc_device_id, address, port, bean_id, model_id, name, serial_number) values
  (102, '10.1.2.2', 5060, 'sbcGeneric', 'sbcGenericModel', 'Sbc for SipTrunk', '201122334455');
  
insert into gateway (
  gateway_id, bean_id, model_id, name, anonymous, 
  keep_digits, transform_user_extension, ignore_user_info, address_port, address_transport, 
  sbc_device_id, shared, enabled)
values 
  (1002, 'gwSipTrunk', 'sipTrunkStandard', 'sip trunk', false,
   0,  false, false, 0, 'none', 102,
   false, true);
