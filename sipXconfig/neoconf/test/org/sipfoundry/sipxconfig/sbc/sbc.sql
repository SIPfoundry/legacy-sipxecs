insert into sbc (sbc_id, sbc_device_id, enabled, sbc_type) values
 (1000, 1000, true, 'D'),
 (1001, 1001, true, 'A'),
 (1002, 1002, true, 'A');

 insert into sbc_route_domain (index, sbc_id, domain) values
  (0, 1000, '*.example.org'),
  (1, 1000, '*.example.net');
  
insert into sbc_route_subnet (index, sbc_id, subnet) values
  (0, 1000, '10.1.2.3/24'),
  (0, 1001, '10.1.2.5/24');
