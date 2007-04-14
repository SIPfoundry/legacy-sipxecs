create table ldap_connection (
    ldap_connection_id int4 not null,
    host varchar(255),
    port int4,
    principal varchar(255),
    secret varchar(255),
    cron_schedule_id int4,
    primary key (ldap_connection_id)
);

create table ldap_attr_map (
    ldap_attr_map_id int4 not null,   
    default_group_name varchar(255),
    default_pin varchar(255),
    search_base varchar(255),
    object_class varchar(255),
    filter varchar(255),
    primary key (ldap_attr_map_id)
);

create table ldap_user_property_to_ldap_attr (
    ldap_attr_map_id int4 not null,   
    user_property varchar(255),
    ldap_attr varchar(255),
    primary key (ldap_attr_map_id, user_property)   
);

create table ldap_selected_object_classes (
    ldap_attr_map_id int4 not null,
    object_class varchar(255)
);

create table cron_schedule (
    cron_schedule_id int4 not null,   
    cron_string varchar(255),
    primary key (cron_schedule_id)   
);

alter table ldap_connection
	add constraint ldap_connection_cron_schedule
	foreign key (cron_schedule_id)
	references cron_schedule;

alter table ldap_user_property_to_ldap_attr 
    add constraint fk_ldap_user_property_to_ldap_attr_ldap_attr_map
    foreign key (ldap_attr_map_id)
    references ldap_attr_map;

alter table ldap_selected_object_classes
    add constraint fk_ldap_selected_object_classes_ldap_attr_map
    foreign key (ldap_attr_map_id)
    references ldap_attr_map;
    

-- used for native hibernate ids  
create sequence hibernate_sequence;
