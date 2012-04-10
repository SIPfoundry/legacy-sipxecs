-- drop table firewall_server_group;
create table firewall_server_group (
   firewall_server_group_id int4 not null,      
   name varchar(255) not null unique,
   servers varchar(255) not null,
   primary key (firewall_server_group_id)
);

--drop table firewall_rule;
create table firewall_rule (
   firewall_rule_id int4 not null,
   prioritize boolean default false,
   address_type varchar(32) not null,   
-- either firewall_server_group_id or system_id is set, not both
   firewall_server_group_id int4,
   system_id varchar(16),
   primary key (firewall_rule_id)
);
 
create sequence firewall_rule_seq;
create sequence firewall_server_group_seq;
