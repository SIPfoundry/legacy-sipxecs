insert into region (region_id, name) values
  (1, 'r1'),
  (2, 'r2'),
  (3, 'r3');

insert into location (location_id, name, fqdn, ip_address, region_id) values
  (1, 's1r1', 's1r1.example.org', '1.1.1.1', 1),
  (2, 's2r1', 's2r1.example.org', '1.1.1.2', 1),
  (3, 's1rx', 's1rx.example.org', '1.1.1.3', NULL),
  (4, 's2rx', 's2rx.example.org', '1.1.1.4', NULL),
  (5, 's1r2', 's1r2.example.org', '1.1.1.5', 2),
  (6, 's1r3', 's1r3.example.org', '1.1.1.6', 3);

insert into dns_plan (dns_plan_id, name) values 
   (1, 'typical'),
   (2, 'one region'),
   (3, 'standard'),
   (4, 'four servers unequally');

alter sequence dns_plan_seq restart with 100;
   
insert into dns_group (dns_group_id, dns_plan_id, position) values 
   (1, 1, 0),
   (2, 1, 0),   
   (11, 2, 0),   
   (21, 3, 0),
   (22, 3, 1),   
   (31, 4, 0);
   
alter sequence dns_group_seq restart with 100;
      
insert into dns_target (dns_group_id, percentage, region_id, location_id, basic_id) values 
   (1,  50, NULL, 1, NULL),
   (1,  50, NULL, 2, NULL),
   (2,  100, NULL, NULL, 'O'),   
   (11, 100, 1, NULL, NULL),   
   (21, 100, NULL, NULL, 'L'),
   (22, 100, NULL, NULL, 'O'),   
   (31, 10, NULL, 1, NULL),
   (31, 10, NULL, 2, NULL),
   (31, 10, NULL, 3, NULL),
   (31, 70, NULL, 4, NULL);
   
insert into dns_view (dns_view_id, dns_plan_id, region_id, name, enabled, position) values
   (1, 1, 1, 'v1-p1-r1', true, 0),
   (2, 1, 1, 'v2-p1-r1-off', false, 1),
   (3, 2, 2, 'v3-p2-r2', true, 2);

 alter sequence dns_view_seq restart with 100;

