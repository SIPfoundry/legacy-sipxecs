alter table location add column primary_location boolean;
update location set primary_location = 'false';
alter table location alter column primary_location set default false;