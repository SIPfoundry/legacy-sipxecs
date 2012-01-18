insert into acd_server (acd_server_id, location_id, port) values
 (1001, 1, 8100);

insert into acd_queue (acd_queue_id, name, acd_server_id) values
 (2001, 'q1', 1001),
 (2002, 'q2', 1001);
