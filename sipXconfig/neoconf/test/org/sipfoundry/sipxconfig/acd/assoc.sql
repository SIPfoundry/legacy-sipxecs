--select truncate_all();
insert into acd_server (acd_server_id, location_id, port) values
 (1001, 1, 8100);

insert into acd_queue (acd_queue_id, name, acd_server_id) values
 (3001, 'q1', 1001),
 (3002, 'q2', 1001);
 
 insert into acd_line (acd_line_id, name, acd_server_id) values
  (2001, 'l1', 1001),
  (2002, 'l2', 1001);
 
insert into acd_line_queue (acd_queue_id, acd_line_id) values
 (3001, 2001);
