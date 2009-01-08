alter table location add column primary_location boolean;
alter table "location" alter column primary_location set default false;