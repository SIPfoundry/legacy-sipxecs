
insert into branch (branch_id, name, description) values
  (1000, 'branch1', 'my test branch');
  
insert into gateway (
  gateway_id, bean_id, model_id, name, anonymous, 
  keep_digits, transform_user_extension, ignore_user_info, address_port, address_transport, 
  shared, enabled, branch_id)
values 
  (1003, 'gwGeneric', 'genericGatewayStandard', 'unmanagedGtw', false,
   0,  false, false, 0, 'none', false,
   true, 1000);
