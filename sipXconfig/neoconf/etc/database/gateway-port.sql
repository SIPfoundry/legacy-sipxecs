alter table gateway add column address_port int4;
update gateway set address_port = 0;
alter table gateway add column address_transport varchar(8);
update gateway set address_transport = 'none';

