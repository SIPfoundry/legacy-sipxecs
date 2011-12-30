insert into users (first_name, last_name, user_name, pintoken, user_id, user_type) values
 ('sam', '', 		'alpha', 'x', 1001, 'C'),
 ('addie', '', 		'beta', 'x', 1002, 'C'),
 ('jessica', '', 	'charlie', 'x', 1003, 'C'),
 ('matt', '', 		'delta', 'x', 1004, 'C'),
 ('snoglish', '', 	'elephant', 'x', 1005, 'C'),
 ('jayne', '', 		'frank', 'x', 1006, 'C'),
 ('logo', '', 		'gogo', 'x', 1007, 'C'),
 ('michael', '', 	'horatio', 'x', 1008, 'C'),
 ('eric', '', 		'janus', 'x', 1009, 'C'),
 ('hogwash', 		'Mamba', 'kyle', 'x', 1010, 'C');

insert into user_alias (user_id, alias) values
 (1001, 'zogu'),
 (1001, 'toga'),
 (1004, 'moon unit'),
 (1005, 'dweezil'),
 (1006, 'yogurt');

insert into value_storage (value_storage_id) values
 (1001),(1002), (1003);

insert into group_storage (group_id, name, weight) values
 (1001, 'x', 1),
 (1002, 'y', 2),
 (1003, 'z', 3);
 
insert into user_group (user_id, group_id) values
 (1001, 1001),
 (1001, 1002),
 (1002, 1002),
 (1004, 1003),
 (1007, 1003);
