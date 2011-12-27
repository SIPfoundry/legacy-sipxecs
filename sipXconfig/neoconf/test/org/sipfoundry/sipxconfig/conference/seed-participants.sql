delete from users where user_name != 'superadmin';
delete from location;
delete from meetme_conference;
delete from meetme_bridge;

insert into users (user_id, user_name, first_name, last_name, user_type) values
	(1002, 'test1002', 'Maxim', 'Afinogenov', 'C'),
    (1003, 'test1003', 'Ilya', 'ln3', 'C'),
    (1004, 'test1004', 'fn1004', 'ln4', 'C'),
    (1005, 'test1005', 'fn1005', 'ln5', 'C');

insert into location (location_id, name, fqdn, primary_location) values
	(1000, 'b1_server', 'host.example.com', true),
	(1001, 'b2_server', 'host2.example.com', false);

insert into meetme_bridge (meetme_bridge_id, location_id) values
	(2005, 1000),
	(2006, 1001);

insert into meetme_conference (meetme_bridge_id, meetme_conference_id, name, extension,  did, owner_id, enabled) values
     (2005, 3001, 'conf_name_3001',			'1699', '123456789', 1002, true),
     (2005, 3002, 'conf_name_3002', 		'1700', null, 1002, true),
     (2006, 3003, 'conf_name_3003', 		'1701', null, 1002, true),
     (2006, 3004, 'conference_name_3004', 	'1703', null, 1003, true),
     (2006, 3005, 'conference_no_owner', 	'1704', null, null, true);
