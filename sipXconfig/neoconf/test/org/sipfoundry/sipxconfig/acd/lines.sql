insert into acd_server (acd_server_id, location_id, port) values
 (1001, 1, 8100);

insert into acd_line (acd_line_id, name, acd_server_id, extension, did) values
 (2001, 'l1', 1001, '101', '123456789'),
 (2002, 'l2', 1001, '102', null);
