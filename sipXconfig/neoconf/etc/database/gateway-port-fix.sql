-- rename enumerations for org.sipfoundry.sipxconfig.gateway.AddressTransport
update gateway set address_transport = 'none' where address_transport is null or address_transport = '';
update gateway set address_transport = 'udp' where address_transport = 'UDP';
update gateway set address_transport = 'tcp' where address_transport = 'TCP';

alter table gateway alter column address_transport set not null;
alter table gateway alter column address_transport set default 'none';
