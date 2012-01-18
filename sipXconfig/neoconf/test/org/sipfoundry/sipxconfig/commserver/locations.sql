insert into location (location_id, name, ip_address, fqdn, primary_location, 
  use_stun, public_tls_port, registered, start_rtp_port, stop_rtp_port,
  stun_interval) values
  (1001, 'Primary', '10.1.1.1', 'primary.exampl.com', true, 
   false, 5161, true, 30000, 31000, 60),
  (1002, 'Secondary', '10.1.1.1', 'remote.exampl.com', false, 
   false, 5161, true, 30000, 31000, 60);
