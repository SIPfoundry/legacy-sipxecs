insert into location (location_id, name, ip_address, fqdn, primary_location, 
  use_stun, public_tls_port, registered, start_rtp_port, stop_rtp_port,
  stun_interval) values
  (1001, 'Primary', '10.1.1.1', 'primary.exampl.com', true, 
   false, 5161, true, 30000, 31000, 60);
   
insert into permission (permission_id, label, description, default_value) values
 (1001, 'bongoLabel', 'bongoDescription', 'FALSE'),
 (1002, 'kukuLabel', 'kukuDescription', 'TRUE');
 
