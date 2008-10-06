-- old addres column is now used to keep fully qualified domain name of the server
-- new ip_address column added to keep ip_address

alter table location rename column address to fqdn;
alter table location add column ip_address varchar(255);

-- all locations have to be in the same sip_domain
alter table location drop column sip_domain;
