alter table gateway add column enabled boolean;
update gateway set enabled=true;
alter table gateway alter column enabled set not null;