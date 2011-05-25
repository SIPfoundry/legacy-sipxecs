-- add colums for media server type and hostname
alter table internal_dialing_rule add column media_server_type varchar(255);
alter table internal_dialing_rule add column media_server_hostname varchar(255);