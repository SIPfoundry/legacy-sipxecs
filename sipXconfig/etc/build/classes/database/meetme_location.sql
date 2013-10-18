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
