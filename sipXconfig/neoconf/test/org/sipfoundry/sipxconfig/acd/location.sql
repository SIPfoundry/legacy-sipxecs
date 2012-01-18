insert into location (location_id, name, ip_address, fqdn, primary_location) values
  (1, 'Config Server, Media Server and Comm Server', '192.168.0.26', 'localhost', true);

insert into feature_local (feature_id, location_id) values
  ('acd', 1);