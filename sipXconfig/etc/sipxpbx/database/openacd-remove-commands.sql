delete from freeswitch_action where freeswitch_condition_id in
	(select freeswitch_condition_id from freeswitch_condition where freeswitch_ext_id in 
		(select freeswitch_ext_id from freeswitch_extension where name = 'login')
	);
delete from freeswitch_condition where freeswitch_ext_id in (select freeswitch_ext_id from freeswitch_extension where name = 'login');
delete from freeswitch_extension where name = 'login';

delete from freeswitch_action where freeswitch_condition_id in 
	(select freeswitch_condition_id from freeswitch_condition where freeswitch_ext_id in
		(select freeswitch_ext_id from freeswitch_extension where name = 'available')
	);
delete from freeswitch_condition where freeswitch_ext_id in (select freeswitch_ext_id from freeswitch_extension where name = 'available');
delete from freeswitch_extension where name = 'available';

delete from freeswitch_action where freeswitch_condition_id in 
	(select freeswitch_condition_id from freeswitch_condition where freeswitch_ext_id in 
		(select freeswitch_ext_id from freeswitch_extension where name = 'logoff')
	);
delete from freeswitch_condition where freeswitch_ext_id in (select freeswitch_ext_id from freeswitch_extension where name = 'logoff');
delete from freeswitch_extension where name = 'logoff';

delete from freeswitch_action where freeswitch_condition_id in 
	(select freeswitch_condition_id from freeswitch_condition where freeswitch_ext_id in 
		(select freeswitch_ext_id from freeswitch_extension where name = 'release')
	);
delete from freeswitch_condition where freeswitch_ext_id in (select freeswitch_ext_id from freeswitch_extension where name = 'release');
delete from freeswitch_extension where name = 'release';