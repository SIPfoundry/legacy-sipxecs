insert into meetme_bridge (meetme_bridge_id, location_id) values
  (1, 1);  

insert into meetme_conference (meetme_conference_id, name, extension, meetme_bridge_id, enabled) values
  (1, 'conf1', '7777', 1, true);
