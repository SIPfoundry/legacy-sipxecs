alter table location add column public_tls_port integer;
update location set public_tls_port=5061;
alter table location alter column public_tls_port set not null;
alter table location alter column public_tls_port set default 5061;
