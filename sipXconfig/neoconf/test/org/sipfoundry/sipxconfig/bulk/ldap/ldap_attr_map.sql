insert into ldap_attr_map (ldap_attr_map_id, default_group_name, default_pin, object_class, search_base, filter) values
 (1000, 'dgn', 'dp', 'person', '', 'filter');

insert into ldap_user_property_to_ldap_attr (ldap_attr_map_id, user_property, ldap_attr) values
 (1000, 'username', 'uid'),
 (1000, 'firstname', 'cn');

 insert into ldap_selected_object_classes (ldap_attr_map_id, object_class) values
 (1000, 'abc'),
 (1000, 'def');
