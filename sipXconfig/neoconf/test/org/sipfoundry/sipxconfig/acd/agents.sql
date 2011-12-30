insert into users (user_id, user_name, user_type) values
  (1000, 'testuser', 'C');

insert into acd_server (acd_server_id, location_id, port) values
 (1001, 1, 8100);

insert into acd_queue (acd_queue_id, name, acd_server_id) values
 (2001, 'q1', 1001),
 (2002, 'q2', 1001);
 
insert into acd_agent (acd_agent_id, user_id, acd_server_id) values
 (3001, 1000, 1001),
 (3002, 1000, 1001),
 (3003, 1000, 1001),
 (3004, 1000, 1001);

insert into acd_queue_agent (acd_agent_id, acd_queue_id, agent_position) values
 (3001, 2002, 0),
 (3002, 2002, 1),
 (3003, 2002, 2),
 (3001, 2001, 0);

insert into acd_agent_queue (acd_agent_id, acd_queue_id, queue_position) values
 (3001, 2002, 1),
 (3002, 2002, 0),
 (3003, 2002, 0),
 (3001, 2001, 0);
