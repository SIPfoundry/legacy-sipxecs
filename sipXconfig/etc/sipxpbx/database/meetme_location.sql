create or replace function migrate_conferences_seed() returns integer as $$
begin    
	-- This function is just meant to be run manually to seed some test data to
	-- test migrate_conferences() functions. It is meant to be executed on
	-- a sipxecs 4.4 schema
	
	insert into location (location_id, name, fqdn) 
	  values (1, 'x', 'x.example.org'), (2, 'y', 'y.example.org');
	  
	insert into sipx_service (sipx_service_id, bean_id)
	  values (1000, 'sipxFreeswitchService'), (2000, 'bogus');
	
	insert into location_specific_service (location_specific_service_id, location_id, sipx_service_id) 
	  values (10, 1, 1000),  (20, 2, 1000);
	  
	insert into meetme_bridge (meetme_bridge_id, location_specific_service_id) 
	  values (100, 10), (200, 20);
    	
	return 1;
end;
$$ language plpgsql;


create or replace function migrate_conferences() returns integer as $$
begin    
	alter table meetme_bridge add column location_id integer;
	update meetme_bridge set location_id = ( 
	  select ls.location_id 
	    from location_specific_service as ls, sipx_service as s
	      where
	        meetme_bridge.location_specific_service_id = ls.location_specific_service_id 
	        and ls.sipx_service_id = s.sipx_service_id
	        and s.bean_id = 'sipxFreeswitchService'
	     );

    alter table meetme_bridge alter column location_id set not null;
    alter table meetme_bridge drop column location_specific_service_id;
    	
	return 1;
end;
$$ language plpgsql;

select migrate_conferences();
