alter table park_orbit add column location_id integer;
alter table meetme_bridge alter column location_id set not null;
update park_orbit set location_id=(select location_id from feature_local where feature_id='park');
