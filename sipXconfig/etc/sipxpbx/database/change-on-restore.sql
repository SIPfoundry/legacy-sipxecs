create or replace function change_domain_on_restore(new_domain text) returns void as $$
declare
    old_domain text;
begin
    SELECT name into old_domain from domain;
    update location set fqdn = replace(fqdn, old_domain, new_domain);
    update domain_alias set alias = replace(alias, old_domain, new_domain);
    update domain set name = new_domain, sip_realm = new_domain;
end;
$$ language plpgsql;


create or replace function change_primary_fqdn_on_restore(new_fqdn text) returns void as $$
declare
    old_fqdn text;
begin
    SELECT fqdn from location into old_fqdn where primary_location = TRUE;
    update domain_alias set alias = new_fqdn where alias = old_fqdn;
    update location set fqdn = new_fqdn where primary_location = TRUE;
end;
$$ language plpgsql;


create or replace function change_primary_ip_on_restore(new_ip text) returns void as $$
declare
    old_ip text;
begin
    SELECT ip_address from location into old_ip where primary_location = TRUE;
    update domain_alias set alias = new_ip where alias = old_ip;
    update location set ip_address = new_ip where primary_location = TRUE;
end;
$$ language plpgsql;


-- use brute force to test all PIN combinations from "0" to "9999" where the
-- upper bounds is set by max_len.  If PIN cannot be "cracked" because the user
-- had more digits than max_len or the PIN actually contained letters, then set
-- all of the "uncracked" PINs to a specfic value.    
--
--  Very crude report of pins that were restore /var/lib/pgsql/data/pg_log
create or replace function uncover_pin_on_restore(reset_pin text, max_len integer) returns void as $$
declare
    realm text;
    max_pin integer;
    pin integer;
    candidate text;
    hash text;
    new_pin text;
    my_user record;
begin
    SELECT sip_realm from domain into realm;   
    raise NOTICE 'realm is %', realm;
	for my_user in select user_name, pintoken from users loop
	    new_pin := NULL;
	    <<cracked>>
		for pin_len in 1..max_len loop
			max_pin := (10 ^ pin_len) - 1;			
			for pin in 0..max_pin loop
				candidate := lpad('' || pin, pin_len, '0');
				hash := md5(my_user.user_name || ':' || realm || ':' || candidate);
				if hash = my_user.pintoken then
				    new_pin := candidate;
					exit cracked;
				end if;
			end loop;
		end loop;
		
		if new_pin is NULL then
  		  new_pin := reset_pin;
  		  raise NOTICE 'RESET PIN % for user %', my_user.user_name;
		end if;
		
		update users set pintoken = md5(my_user.user_name || ':' || new_pin) where user_name = my_user.user_name;
	end loop;
end;	
$$ language plpgsql;
