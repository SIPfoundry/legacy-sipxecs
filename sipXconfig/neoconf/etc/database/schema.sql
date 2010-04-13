/*
 * Unmapped hibernate tables
 */
create table version_history(
  version int4 not null primary key,
  applied date not null
);

/**
 * CHANGE VERSION HERE ----------------------------> X <------------------
 *
 * For sipXconfig v3.0, the database version is 1.
 * For sipXconfig v3.2, the database version is 2.
 * For sipXconfig v3.3-r6543, the database version is 3.
 * For sipXconfig v3.5-r7552, the database version is 4.
 * For sipXconfig v3.7-r7934, the database version is 5.
 * For sipXconfig v3.8-r10357, the database version is 6.
 * For sipXconfig v3.9-r11768, the database version is 7.
 * For sipXconfig v3.10.3-r11768, the database version is 8.
 * For sipXconfig v3.11, the database version is 9.
 * For sipXconfig v4.0.2, the database version is 10.
 * For sipXconfig v4.1.8, the database version is 11.
 */
insert into version_history (version, applied) values (1, now());
insert into version_history (version, applied) values (2, now());
insert into version_history (version, applied) values (3, now());
insert into version_history (version, applied) values (4, now());
insert into version_history (version, applied) values (5, now());
insert into version_history (version, applied) values (6, now());
insert into version_history (version, applied) values (7, now());
insert into version_history (version, applied) values (8, now());
insert into version_history (version, applied) values (9, now());
insert into version_history (version, applied) values (10, now());
insert into version_history (version, applied) values (11, now());

create table patch(
  name varchar(32) not null primary key
);
/*
 * hibernate tables
 */
create table phone (
   phone_id int4 not null,
   description varchar(255),
   serial_number varchar(255) not null unique,
   bean_id varchar(255),
   value_storage_id int4,
   model_id varchar(64) not null,
   device_version_id varchar(32),
   primary key (phone_id)
);

create table dial_pattern (
   custom_dialing_rule_id int4 not null,
   element_prefix varchar(255),
   element_digits int4,
   index int4 not null,
   primary key (custom_dialing_rule_id, index)
);
create table international_dialing_rule (
   international_dialing_rule_id int4 not null,
   international_prefix varchar(255),
   primary key (international_dialing_rule_id)
);
create table setting_value (
   value_storage_id int4 not null,
   value varchar(255) not null,
   path varchar(255) not null,
   primary key (value_storage_id, path)
);
create table line_group (
   line_id int4 not null,
   group_id int4 not null,
   primary key (line_id, group_id)
);
create table dialing_rule (
   dialing_rule_id int4 not null,
   name varchar(255) not null unique,
   description varchar(255),
   enabled bool,
   position int4,
   dial_plan_id int4,
   schedule_id int4,
   primary key (dialing_rule_id)
);
create table auto_attendant (
   auto_attendant_id int4 not null,
   name varchar(255) unique,
   extension varchar(255),
   prompt varchar(255),
   system_id varchar(255),
   description varchar(255),
   value_storage_id int4,
   group_id int4,
   primary key (auto_attendant_id)
);
create table attendant_group (
   auto_attendant_id int4 not null,
   group_id int4 not null,
   primary key (auto_attendant_id, group_id)
);
create table park_orbit (
   park_orbit_id int4 not null,
   orbit_type char(1) not null,
   music varchar(255),
   enabled bool,
   name varchar(255) unique,
   extension varchar(255),
   description varchar(255),
   value_storage_id int4,
   group_id int4,
   primary key (park_orbit_id)
);
create table group_storage (
   group_id int4 not null,
   name varchar(255),
   description varchar(255),
   resource varchar(255),
   weight int4,
   branch_id int4,
   primary key (group_id)
);
create table user_ring (
   user_ring_id int4 not null,
   position int4,
   expiration int4,
   ring_type varchar(255),
   call_group_id int4 not null,
   user_id int4 not null,
   primary key (user_ring_id)
);

create table gateway (
   gateway_id int4 not null,
   bean_id varchar(255) not null,
   name varchar(255) unique,
   address varchar(255),
   description varchar(255),
   serial_number varchar(255),
   value_storage_id int4,
   model_id varchar(64) not null,
   device_version_id varchar(32),
   prefix varchar(255),
   default_caller_alias varchar(255),
   anonymous boolean not null default false,
   ignore_user_info boolean not null default false,
   transform_user_extension boolean not null default false,
   add_prefix varchar(255),
   keep_digits integer not null default 0,
   address_port int4 not null default 0,
   address_transport varchar(8) not null default 'none',
   sbc_device_id int4,
   enable_caller_id boolean not null default false,
   caller_id varchar(255),
   display_name varchar(255),
   url_parameters varchar(255),
   shared boolean not null,
   branch_id int4,
   enabled boolean not null,
   primary key (gateway_id)
);

create table daily_backup_schedule (
   daily_backup_schedule_id int4 not null,
   enabled bool,
   time_of_day timestamp,
   scheduled_day varchar(255),
   backup_plan_id int4,
   primary key (daily_backup_schedule_id)
);
create table job (
   job_id int4 not null,
   type int4,
   status char(1),
   start_time_string varchar(255),
   details varchar(255),
   progress varchar(255),
   exception_message varchar(255),
   primary key (job_id)
);
create table group_weight (
   weight int4 not null,
   primary key (weight)
);
create table line (
   line_id int4 not null,
   phone_id int4 not null,
   position int4,
   user_id int4,
   value_storage_id int4,
   primary key (line_id)
);
create table value_storage (
   value_storage_id int4 not null,
   primary key (value_storage_id)
);
create table local_dialing_rule (
   local_dialing_rule_id int4 not null,
   external_len int4,
   pstn_prefix varchar(255),
   primary key (local_dialing_rule_id)
);
create table attendant_menu_item (
   auto_attendant_id int4 not null,
   action varchar(255),
   parameter varchar(255),
   dialpad_key varchar(255) not null,
   primary key (auto_attendant_id, dialpad_key)
);
create table custom_dialing_rule (
   custom_dialing_rule_id int4 not null,
   call_pattern_digits varchar(255),
   call_pattern_prefix varchar(255),
   primary key (custom_dialing_rule_id)
);
create table call_group (
   call_group_id int4 not null,
   enabled bool,
   name varchar(255) unique,
   extension varchar(255),
   description varchar(255),
   fallback_destination varchar(255),
   voicemail_fallback boolean not null default false,
   user_forward boolean not null default true,
   sip_password varchar(255),
   primary key (call_group_id)
);
create table phone_group (
   phone_id int4 not null,
   group_id int4 not null,
   primary key (phone_id, group_id)
);
create table dialing_rule_gateway (
   dialing_rule_id int4 not null,
   gateway_id int4 not null,
   index int4 not null,
   primary key (dialing_rule_id, index)
);
create table emergency_dialing_rule (
   emergency_dialing_rule_id int4 not null,
   optional_prefix varchar(255),
   emergency_number varchar(255),
   primary key (emergency_dialing_rule_id)
);
create table backup_plan (
   backup_plan_id int4 not null,
   limited_count int4,
   configs bool,
   voicemail bool,
   email_address varchar(255),
   backup_type char not null,
   ftp_configuration_id int4,
   primary key (backup_plan_id)
);
create table dial_plan (
   dial_plan_id int4 not null,
   type character varying(255),
   primary key (dial_plan_id)
);
create table ring (
   ring_id int4 not null,
   number varchar(255),
   position int4,
   expiration int4,
   ring_type varchar(255),
   user_id int4 not null,
   enabled bool not null default true,
   schedule_id integer,
   primary key (ring_id)
);
create table users (
   user_id int4 not null,
   first_name varchar(255),
   pintoken varchar(255),
   sip_password varchar(255),
   last_name varchar(255),
   user_name varchar(255) not null unique,
   value_storage_id int4,
   address_book_entry_id integer,
   is_shared boolean not null default false,
   branch_id int4,
   user_type char(1),
   primary key (user_id)
);
create table custom_dialing_rule_permission (
   custom_dialing_rule_id int4 not null,
   permission varchar(255)
);
create table internal_dialing_rule (
   internal_dialing_rule_id int4 not null,
   auto_attendant_id int4,
   local_extension_len int4,
   voice_mail varchar(255),
   voice_mail_prefix varchar(255),
   aa_aliases varchar(255),
   media_server_type varchar(255),
   media_server_hostname varchar(255),
   primary key (internal_dialing_rule_id)
);
create table long_distance_dialing_rule (
   international_dialing_rule_id int4 not null,
   area_codes varchar(255),
   external_len int4,
   long_distance_prefix varchar(255),
   permission varchar(255),
   pstn_prefix varchar(255),
   pstn_prefix_optional bool,
   long_distance_prefix_optional bool,
   primary key (international_dialing_rule_id)
);
create table initialization_task (
  name varchar(255) not null primary key
);
create table user_group(
   user_id int4 not null,
   group_id int4 not null,
   primary key (user_id, group_id)
);
create table user_alias (
  user_id int4 not null,
  alias varchar(255) not null unique,
  primary key (user_id, alias)
);
create table meetme_participant (
    meetme_participant_id int4 not null,
    enabled bool,
    value_storage_id int4,
    user_id int4 not null,
	meetme_conference_id int4 not null,
    primary key (meetme_participant_id)
);
create table meetme_conference (
    meetme_conference_id int4 not null,
    enabled bool,
    name varchar(255) not null,
    description varchar(255),
    extension varchar(255),
    value_storage_id int4,
    meetme_bridge_id int4 not null,
    owner_id int4,
    primary key (meetme_conference_id)
);
create table meetme_bridge (
    meetme_bridge_id int4 not null,
    value_storage_id int4,
    location_specific_service_id integer,
    primary key (meetme_bridge_id)
);

create table extension_pool (
  extension_pool_id int4 not null,
  enabled boolean not null,
  name varchar(255) unique not null,
  first_extension int4,
  last_extension int4,
  next_extension int4,
  primary key (extension_pool_id)
);

create table upload(
  upload_id int4 not null primary key,
  name varchar(255) not null unique,
  deployed bool not null,
  bean_id varchar(32) not null,
  specification_id varchar(32),
  value_storage_id int4,
  description varchar(255)
);

create table attendant_dialing_rule (
   attendant_dialing_rule_id int4 not null,
   attendant_aliases varchar(255),
   extension varchar(255),
   after_hours_attendant_id int4,
   after_hours_attendant_enabled bool not null,
   
   holiday_attendant_id int4,
   holiday_attendant_enabled bool not null,
   
   working_time_attendant_id int4,
   working_time_attendant_enabled bool not null,
   
   primary key (attendant_dialing_rule_id)
);

create table attendant_working_hours (
   attendant_dialing_rule_id int4 not null,
   index int4 not null,
   enabled bool not null,
   day varchar(255),
   start timestamp,
   stop timestamp,
   primary key (attendant_dialing_rule_id, index)
);

create table holiday_dates (
   attendant_dialing_rule_id int4 not null,
   position int4 not null,
   date timestamp,
   primary key (attendant_dialing_rule_id, position)
);
  
create table site_to_site_dialing_rule (
   site_to_site_dialing_rule_id int4 not null,
   call_pattern_digits varchar(255),
   call_pattern_prefix varchar(255),
   primary key (site_to_site_dialing_rule_id)
);

create table site_to_site_dial_pattern (
   site_to_site_dialing_rule_id int4 not null,
   element_prefix varchar(255),
   element_digits int4,
   index int4 not null,
   primary key (site_to_site_dialing_rule_id, index)
);

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
    enabled bool not null default false,
    primary key (cron_schedule_id)   
);

create table supervisor(
   user_id int4 not null,
   group_id int4 not null,
   primary key (user_id, group_id)
);  

/*
 * Intercom configuration:
 *   prefix - the prefix for placing an intercom call
 *   timeout (in milliseconds) - how long the phone rings before the call is automatically answered
 *   code - a string sent to the phone that identifies the intercom configuration
 */
create table intercom (
   intercom_id int4 not null,
   enabled bool not null,
   prefix varchar(255) not null unique,
   timeout int4 not null,
   code varchar(255) not null unique,
   primary key (intercom_id)
);

/*
 * intercom_phone_group is a join table that links an intercom configuration
 * to one or more phone groups
 */
create table intercom_phone_group (
   intercom_id int4 not null,
   group_id int4 not null,
   primary key (intercom_id, group_id)
);

create table permission (
  permission_id int4 not null,
  description varchar(255),
  label varchar(255),
  default_value boolean not null,
  primary key (permission_id)
);

create table park_orbit_group (
   park_orbit_id int4 not null,
   group_id int4 not null,
   primary key (park_orbit_id, group_id)
);

create table domain (
   domain_id int4 not null,   
   name varchar(255) not null,
   shared_secret varchar(255),
   sip_realm varchar(255),
   primary key (domain_id)
);

create table domain_alias (
   domain_id int4 not null,
   alias varchar(255) not null,
   primary key (domain_id, alias)   
);

create table phonebook (
   phonebook_id int4 not null,      
   name varchar(255) not null unique,
   description varchar(255),
   members_csv_filename varchar(255),
   members_vcard_filename varchar(255),
   user_id int4,
   show_on_phone boolean default true,
   primary key (phonebook_id)
);

create table phonebook_member (
   phonebook_id int4 not null,      
   group_id int4 not null,      
   primary key (phonebook_id, group_id)
);

create table phonebook_consumer (
   phonebook_id int4 not null,      
   group_id int4 not null,      
   primary key (phonebook_id, group_id)
);

create table speeddial (
    speeddial_id int4 not null,
    user_id int4 not null,
    primary key (speeddial_id)
);

create table speeddial_button (
    speeddial_id int4 not null,
    label varchar(255),
    number varchar(255) not null,
    blf boolean not null default false,
    position int4 not null,
    primary key (speeddial_id, position)
);

create table acd_agent (
    acd_agent_id int4 not null,
    value_storage_id int4,
    user_id int4 not null,
    acd_server_id int4 not null,
    primary key (acd_agent_id)
);

create table acd_line (
    acd_line_id int4 not null,
    name varchar(255) not null,
    description varchar(255),
    value_storage_id int4,
    acd_server_id int4 not null,
    extension varchar(255),
    primary key (acd_line_id)
);

create table acd_line_queue (
    acd_line_id int4 not null,
    acd_queue_id int4 not null,
    primary key (acd_line_id)
);

create table acd_queue (
    acd_queue_id int4 not null,
    name varchar(255) not null,
    description varchar(255),
    value_storage_id int4,
    acd_server_id int4 not null,
    overflow_queue_id int,
    primary key (acd_queue_id)
);

create table acd_queue_agent (
    acd_queue_id int4 not null,
    acd_agent_id int4 not null,
    agent_position int4,
    primary key (acd_queue_id, agent_position)
);

create table acd_agent_queue (
    acd_queue_id int4 not null,
    acd_agent_id int4 not null,
    queue_position int4 not null,
    primary key (acd_agent_id, queue_position)
);

create table acd_server (
    acd_server_id int4 not null,
    host varchar(255),
    port int4,
    description varchar(255),
    value_storage_id int4,
    location_id int4,
    primary key (acd_server_id)
);

create table service(
  service_id int4 not null primary key,
  name varchar(255) not null unique,
  address varchar(255) not null,
  bean_id varchar(32) not null,
  enabled bool not null,
  descriptor_id varchar(32),
  value_storage_id int4,
  description varchar(255)
);

create table fxo_port (
  fxo_port_id int4 not null,
  gateway_id int4 not null,
  position int4,
  value_storage_id int4,
  primary key (fxo_port_id)
);

create table sbc (
    sbc_id int4 not null,
    enabled bool,
    address varchar(255),
    sbc_type char(1) not null,
    sbc_device_id int4,
    primary key (sbc_id)
);

create table sbc_route_domain (
    sbc_id int4 not null,
    domain varchar(255) not null,
    index int4 not null,
    primary key (sbc_id, index)
);

create table sbc_route_subnet (
    sbc_id int4 not null,
    subnet varchar(255) not null,
    index int4 not null,
    primary key (sbc_id, index)
);

-- create schedule table
create table schedule (
  schedule_id integer not null primary key,
  user_id integer,
  name varchar(255) not null,
  description varchar(255),
  group_id int4,
  schedule_type char(1),
  constraint fk_schedule_users foreign key (user_id)
      references users,
  constraint fk_schedule_user_groups foreign key (group_id)
      references group_storage (group_id)
      on update no action on delete no action
);

-- create schedule_hours table
create table schedule_hours
(
  schedule_hours_id integer not null,
  schedule_id integer not null,
  "start" timestamp without time zone,
  "stop" timestamp without time zone,
  "day" varchar(255),
  constraint pk_schedule_hours_id_schedule_id primary key (schedule_hours_id, schedule_id),
  constraint fk_schedule_hours_schedule foreign key (schedule_id)
      references schedule (schedule_id) match full
      on update no action on delete no action
);

create table personal_attendant (
   personal_attendant_id int4 not null,
   user_id int4 not null,
   operator varchar(255),
   language varchar(255),
   override_language boolean not null default false,
   primary key (personal_attendant_id)
);

create table personal_attendant_menu_item (
   personal_attendant_id int4 not null,
   action varchar(255),
   parameter varchar(255),
   dialpad_key varchar(255) not null,
   primary key (personal_attendant_id, dialpad_key)
);

create table localization (
   localization_id int4 not null,
   "region" character varying(255),
   "language" character varying(255),
   primary key (localization_id)
);

create table paging_group (
  paging_group_id integer not null,
  page_group_number integer not null,
  description character varying(255),
  enabled boolean not null default false,
  sound character varying(255) not null,
  timeout integer not null default 60,
  constraint paging_group_pkey primary key (paging_group_id)
);

create table user_paging_group
(
  paging_group_id integer not null,
  user_id integer not null,
  constraint user_paging_group_pkey primary key (paging_group_id, user_id),
  constraint paging_group_fk1 foreign key (paging_group_id)
      references paging_group (paging_group_id) match simple
      on update no action on delete no action,
  constraint paging_group_fk2 foreign key (user_id)
      references users (user_id) match simple
      on update no action on delete no action
);

create table paging_server (
  paging_server_id integer not null,
  prefix character varying(255) not null,
  sip_trace_level character varying(255) not null,
  constraint paging_server_pkey primary key (paging_server_id)
);

create table sbc_device (
   sbc_device_id int4 not null,
   bean_id varchar(255) not null,
   name varchar(255) unique,
   address varchar(255),
   description varchar(255),
   serial_number varchar(255),
   value_storage_id int4,
   model_id varchar(64) not null,
   device_version_id varchar(32),
   port int4 not null default 5060,
   branch_id int4,
   primary key (sbc_device_id)
);

create table special_user (
  special_user_id int4 not null,
  user_type varchar(255),
  sip_password varchar(255),
  primary key (special_user_id)
);

create table location (
  location_id int4 not null,
  name varchar(255) not null,
  fqdn varchar(255) not null,
  ip_address varchar(255),
  password varchar(255),
  primary_location boolean not null default false,
  registered boolean not null default false,
  use_stun boolean not null default true,
  stun_address varchar(255),
  stun_interval integer not null default 60,
  public_address varchar(255),
  public_port integer not null default 5060,
  start_rtp_port integer not null default 30000,
  stop_rtp_port integer not null default 31000,
  branch_id int4,
  primary key (location_id)
);

create table sipx_service(
  sipx_service_id int4 not null primary key,
  bean_id varchar(32) not null,
  value_storage_id int4
);

create table location_specific_service(
  location_specific_service_id int4 not null primary key,
  location_id int4 not null,
  sipx_service_id int4 not null,
  enable_on_next_upgrade boolean not null default false
);

create table location_bundle (
  location_id int4 not null,
  bundle_bean varchar(255) not null,
  primary key (location_id, bundle_bean)
);

create table ftp_configuration (
  id int4 not null,
  host varchar(255),
  user_id varchar(255),
  "password" varchar(255),
  constraint ftp_configuration_pkey primary key(id)
);

create table discovered_devices (
  mac_address varchar(255) not null,
  ip_address varchar(255) not null,
  vendor varchar(255) not null,
  last_seen timestamp default now(),
  primary key (mac_address)
);

create table alarm_server (
  alarm_server_id integer not null,
  alarm_notification_enabled boolean,
  from_email_address varchar(255),
  constraint alarm_server_pkey primary key (alarm_server_id)
);

create table timezone
(
  timezone_id integer not null,
  gmt_offset integer,
  dstsavings_enabled boolean,
  dst_savings integer,
  start_month integer,
  start_week integer,
  start_day_of_week integer,
  start_time integer,
  stop_month integer,
  stop_week integer,
  stop_day_of_week integer,
  stop_time integer,
  constraint timezone_pkey primary key (timezone_id)
);

create table speeddial_group (
    speeddial_id int4 not null,
    group_id int4 not null,
    primary key (speeddial_id)
);

create table speeddial_group_button (
    speeddial_id int4 not null,
    label varchar(255),
    number varchar(255) not null,
    blf boolean not null default false,
    position int4 not null,
    primary key (speeddial_id, position)
);

create table address_book_entry
(
  address_book_entry_id integer not null,
  cell_phone_number character varying(255),
  home_phone_number character varying(255),
  assistant_name character varying(255),
  assistant_phone_number character varying(255),
  fax_number character varying(255),
  location character varying(255),
  company_name character varying(255),
  job_title character varying(255),
  job_dept character varying(255),
  im_id character varying(255),
  alternate_im_id character varying(255),
  im_display_name character varying(255),
  use_branch_address boolean DEFAULT false NOT NULL,
  im_password varchar(255),
  email_address character varying(255),
  alternate_email_address character varying(255),
  home_address_id integer,
  office_address_id integer,
  branch_address_id integer,
  constraint address_book_entry_pkey primary key (address_book_entry_id)
);

create table phonebook_file_entry (
  phonebook_file_entry_id integer not null,
  first_name character varying(255),
  last_name character varying(255),
  phone_number character varying(255),
  address_book_entry_id integer,
  phonebook_id integer not null,
  phonebook_entry_type char(1),
  google_account_id varchar(255),
  internal_id varchar(255),
  constraint phonebook_file_entry_pkey primary key (phonebook_file_entry_id)
);

create table attendant_special_mode (
    attendant_special_mode_id int4 not null,
    enabled boolean not null default false,
    auto_attendant_id int4,
    primary key (attendant_special_mode_id)
);

create table branch (
  branch_id int4 not null,
  name varchar(255) unique,
  description varchar(255),
  phone_number varchar(255),
  fax_number varchar(255),
  address_id integer,
  primary key (branch_id)
);

create table private_user_key (
  private_user_key_id int4 not null,
  key varchar(255) unique,
  user_id int4,
  primary key (private_user_key_id)
);

create table alarm_group (
  alarm_group_id integer not null,
  name varchar(255) not null,
  description character varying(255),
  enabled boolean not null default false,
  constraint alarm_group_pkey primary key (alarm_group_id)
);

create table alarm_group_smscontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  index integer not null,
  constraint group_smscontacts_pkey primary key (alarm_group_id, index)
);

create table alarm_group_emailcontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  index integer not null,
  constraint group_emailcontacts_pkey primary key (alarm_group_id, index)
);

create table user_alarm_group
(
  alarm_group_id integer not null,
  user_id integer not null,
  constraint user_alarm_group_pkey primary key (alarm_group_id, user_id)
);

create table alarm_code (
  alarm_seq_id integer not null,
  alarm_code_id varchar(255) not null,
  email_group varchar(255) not null,
  min_threshold integer not null,
  constraint alarm_code_pkey primary key (alarm_seq_id)
);

create table tls_peer
(
  tls_peer_id integer NOT NULL,
  "name" character varying(255) NOT NULL,
  internal_user_id integer,
  constraint tls_peer_pkey primary key (tls_peer_id)
);

create table google_domain (
  google_domain_id integer not null,
  domain_name character varying(255),
  constraint google_domain_pkey primary key (google_domain_id)
);

create table general_phonebook_settings (
   general_phonebook_settings_id int4 not null,
   value_storage_id int4,
   primary key (general_phonebook_settings_id)
);

create table alarm_group_snmpcontacts (
  alarm_group_id integer not null,
  address varchar(255) not null,
  port int4,
  community_string varchar(255),
  index integer,
  constraint group_snmpcontacts_pkey primary key (alarm_group_id, index)
);

create table address
(
  address_id integer not null,
  street character varying(255),
  zip character varying(255),
  country character varying(255),
  state character varying(255),
  city character varying(255),
  office_designation character varying(255),
  constraint address_pkey primary key (address_id)
);

/*
 * Foreign key constraints
 */
 
alter table intercom_phone_group
  add constraint intercom_phone_group_fk1
  foreign key (intercom_id)
  references intercom;

alter table intercom_phone_group
  add constraint intercom_phone_group_fk2
  foreign key (group_id)
  references group_storage;

alter table supervisor add constraint supervisor_fk1 foreign key (user_id) references users;
alter table supervisor add constraint supervisor_fk2 foreign key (group_id) references group_storage;

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

alter table upload add constraint upload_value_storage foreign key (value_storage_id) references value_storage;

alter table phone add constraint FK65B3D6ECB50FCED foreign key (value_storage_id) references value_storage;
alter table dial_pattern add constraint FK8D4D2DC1454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table international_dialing_rule add constraint FKE5D682BA7DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
alter table setting_value add constraint FKBB1806C2CB50FCED foreign key (value_storage_id) references value_storage;
alter table line_group add constraint FK8A14274A8B4D46 foreign key (line_id) references line;
alter table line_group add constraint FK8A142741E2E76DB foreign key (group_id) references group_storage;
alter table dialing_rule add constraint FK3B60F0A99F03EC22 foreign key (dial_plan_id) references dial_plan;
alter table group_storage add constraint FK92BDF0BB1E2E76DB foreign key (group_id) references value_storage;
alter table group_storage add constraint uqc_group_storage_name unique (name, resource);
alter table user_ring add constraint FK143BDE242A05F79C foreign key (call_group_id) references call_group;
alter table user_ring add constraint FK143BDE24F73AEE0F foreign key (user_id) references users;

alter table gateway
  add constraint FKF4BA4644CB50FCED
  foreign key (value_storage_id)
  references value_storage;

alter table gateway
  add constraint fk_sbc_device_id
  foreign key (sbc_device_id)
  references sbc_device(sbc_device_id) match full;

alter table daily_backup_schedule add constraint FK5518A4EFE76E474 foreign key (backup_plan_id) references backup_plan;
alter table line add constraint FK32AFF4CB50FCED foreign key (value_storage_id) references value_storage;
alter table line add constraint FK32AFF4B3B3158C foreign key (phone_id) references phone;
alter table line add constraint FK32AFF4F73AEE0F foreign key (user_id) references users;
alter table local_dialing_rule add constraint FK365020FD76B0539D foreign key (local_dialing_rule_id) references dialing_rule;
alter table attendant_menu_item add constraint FKD3F02D415BEFFE1D foreign key (auto_attendant_id) references auto_attendant;
alter table custom_dialing_rule add constraint FKB189AEB7454433A3 foreign key (custom_dialing_rule_id) references dialing_rule;
alter table phone_group add constraint FKD5244C6EB3B3158C foreign key (phone_id) references phone;
alter table phone_group add constraint FKD5244C6E1E2E76DB foreign key (group_id) references group_storage;
alter table dialing_rule_gateway add constraint FK65E824AE38F854F6 foreign key (gateway_id) references gateway;
alter table dialing_rule_gateway add constraint FK65E824AEF6075471 foreign key (dialing_rule_id) references dialing_rule;
alter table emergency_dialing_rule add constraint FK7EAEE897444E2DC3 foreign key (emergency_dialing_rule_id) references dialing_rule;
alter table ring add constraint FK356A30F73AEE0F foreign key (user_id) references users;
alter table users add constraint FK6A68E08F73AEE0F foreign key (user_id) references users;
alter table custom_dialing_rule_permission add constraint FK8F3EE457454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table internal_dialing_rule add constraint FK5D102EEB5BEFFE1D foreign key (auto_attendant_id) references auto_attendant;
alter table internal_dialing_rule add constraint FK5D102EEBDE4556EF foreign key (internal_dialing_rule_id) references dialing_rule;
alter table long_distance_dialing_rule add constraint FKA10B67307DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
alter table users add constraint user_fk1 foreign key (value_storage_id) references value_storage;
alter table user_group add constraint user_group_fk1 foreign key (user_id) references users;
alter table user_group add constraint user_group_fk2 foreign key (group_id) references group_storage;
alter table user_alias add constraint user_alias_fk1 foreign key (user_id) references users;

alter table site_to_site_dialing_rule
    add constraint site_to_site_dialing_rule_dialing_rule
    foreign key (site_to_site_dialing_rule_id)
    references dialing_rule;

alter table site_to_site_dial_pattern
    add constraint site_to_site_dialing_pattern_dialing_rule
    foreign key (site_to_site_dialing_rule_id)
    references site_to_site_dialing_rule;

alter table meetme_conference
    add constraint fk_meetme_conference_bridge
    foreign key (meetme_bridge_id)
    references meetme_bridge;

alter table meetme_conference
    add constraint fk_owner_id
    foreign key (owner_id)
    references users(user_id) on delete set null;

alter table meetme_participant
    add constraint fk_meetme_participant_conference
    foreign key (meetme_conference_id)
    references meetme_conference;

alter table meetme_participant
    add constraint fk_meetme_participant_user
    foreign key (user_id)
    references users;

alter table meetme_bridge
    add constraint fk_location_specific_service_id
    foreign key(location_specific_service_id)
    references location_specific_service;

alter table attendant_dialing_rule 
  add constraint fk_attendant_dialing_rule_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references dialing_rule;

alter table attendant_working_hours 
  add constraint fk_attendant_working_hours_attendant_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references attendant_dialing_rule;

alter table holiday_dates 
  add constraint fk_holiday_dates_attendant_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references attendant_dialing_rule;
  
alter table attendant_dialing_rule  
  add constraint fk_after_hours_attendant_auto_attendant 
  foreign key (after_hours_attendant_id) 
  references auto_attendant;

alter table attendant_dialing_rule  
  add constraint fk_working_time_attendant_auto_attendant 
  foreign key (working_time_attendant_id) 
  references auto_attendant;  

alter table meetme_conference add constraint conference_name_key unique(name);

alter table phonebook_member add constraint phonebook_member_fk1 foreign key (phonebook_id) references phonebook;
alter table phonebook_member add constraint phonebook_member_fk2 foreign key (group_id) references group_storage;
alter table phonebook_consumer add constraint phonebook_consumer_fk1 foreign key (phonebook_id) references phonebook;
alter table phonebook_consumer add constraint phonebook_consumer_fk2 foreign key (group_id) references group_storage;

alter table speeddial 
    add constraint fk_speeddial_user 
    foreign key (user_id) 
    references users;

alter table speeddial_button 
  add constraint fk_speeddial_button_speeddial
  foreign key (speeddial_id)
  references speeddial;

alter table acd_queue_agent
    add constraint fk_acd_queue_agent_queue_id
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_queue_agent 
    add constraint fk_acd_queue_agent_agent_id 
    foreign key (acd_agent_id) 
    references acd_agent;
    
alter table acd_agent_queue
    add constraint fk_acd_agent_queue_queue_id
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_agent_queue 
    add constraint fk_acd_agent_queue_agent_id 
    foreign key (acd_agent_id) 
    references acd_agent;
    
alter table acd_agent 
    add constraint FKAB953508A53EB3E4 
    foreign key (user_id) 
    references users;
alter table acd_agent 
    add constraint FKAB9535086C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_agent 
    add constraint fk_acd_agent_server 
    foreign key (acd_server_id) 
    references acd_server;
alter table acd_line 
    add constraint FK816CF191D04BF1FA 
    foreign key (acd_server_id) 
    references acd_server;
alter table acd_line 
    add constraint FK816CF1916C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_line_queue 
    add constraint FK1D273CE31DACB05A 
    foreign key (acd_line_id) 
    references acd_line;
alter table acd_line_queue 
    add constraint FK1D273CE3BEE3CFA 
    foreign key (acd_queue_id) 
    references acd_queue;
alter table acd_queue 
    add constraint FKAC7D0B14D04BF1FA 
    foreign key (acd_server_id) 
    references acd_server;
alter table acd_queue 
    add constraint FKAC7D0B146C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_server 
    add constraint FKE5B27DA06C899BCE 
    foreign key (value_storage_id) 
    references value_storage;
alter table acd_server
    add constraint fk_location_id
    foreign key (location_id)
	references location(location_id) match full;

alter table service add constraint service_value_storage foreign key (value_storage_id) references value_storage;

alter table fxo_port 
  add constraint fk_fxo_port_gateway 
  foreign key (gateway_id) 
  references gateway;

alter table fxo_port 
  add constraint fk_fxo_port_value_storage 
  foreign key (value_storage_id) 
  references value_storage;

alter table sbc_route_domain 
    add constraint fk_sbc_route_domain_sbc 
    foreign key (sbc_id) 
    references sbc;

alter table sbc_route_subnet 
    add constraint fk_sbc_route_subnet_sbc 
    foreign key (sbc_id) 
    references sbc;

alter table ring
    add constraint fk_ring_schedule_id
    foreign key (schedule_id)
    references schedule(schedule_id) match full;

alter table personal_attendant
    add constraint  fk_personal_atendant_users
    foreign key (user_id)
    references users;

alter table personal_attendant_menu_item
    add constraint fk_personal_attendant_menu_item_personal_attendant
    foreign key (personal_attendant_id)
    references personal_attendant;

alter table dialing_rule
    add constraint fk_dialing_rule_schedule_id
    foreign key (schedule_id)
    references schedule(schedule_id) match full;

alter table sbc
    add constraint fk_sbc_device_id
    foreign key (sbc_device_id)
	references sbc_device(sbc_device_id) match full;

alter table backup_plan
    add constraint fk_backup_plan_ftp_configuration
    foreign key (ftp_configuration_id)
    references ftp_configuration (id);

alter table location_specific_service
  add constraint fk_location_id
  foreign key (location_id)
  references location;

alter table location_specific_service
  add constraint fk_sipx_service
  foreign key (sipx_service_id)
  references sipx_service;

alter table location_bundle
  add constraint location_bundle_fk
  foreign key (location_id)
  references location;

alter table speeddial_group
  add constraint fk_speeddial_group
  foreign key (group_id)
  references group_storage;

alter table speeddial_group_button
  add constraint fk_speeddial_button_speeddial
  foreign key (speeddial_id)
  references speeddial_group;

alter table users
  add constraint fk_address_book_entry_users_id
  foreign key (address_book_entry_id)
  references address_book_entry;

alter table phonebook_file_entry
  add constraint phonebook_entry_phonebook foreign key (phonebook_id)
  references phonebook;

alter table phonebook_file_entry
  add constraint phonebook_entry_address_book_entry foreign key (address_book_entry_id)
  references address_book_entry;

alter table users add constraint fk_users_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

alter table group_storage add constraint fk_group_storage_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

alter table gateway add constraint fk_gateway_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

alter table location add constraint fk_location_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

alter table sbc_device add constraint fk_sbc_device_branch
  foreign key (branch_id)
  references branch(branch_id) match full
  on delete set null;

alter table private_user_key add constraint fk_private_user_key_user
  foreign key (user_id)
  references users(user_id) match full
  on delete cascade;

alter table phonebook add constraint fk_phonebook_users foreign key (user_id) references users;

alter table user_alarm_group add constraint alarm_group_fk1 foreign key (alarm_group_id)
  references alarm_group (alarm_group_id) match simple
  on update no action on delete no action;

alter table user_alarm_group add constraint alarm_group_fk2 foreign key (user_id)
  references users (user_id) match simple
  on update no action on delete no action;

alter table tls_peer add constraint fk_internal_user foreign key (internal_user_id)
  references users (user_id) match simple
  on update no action on delete no action;

alter table address_book_entry add constraint fk_home_address_id foreign key (home_address_id)
  references address (address_id) match simple
  on update no action on delete no action;

alter table address_book_entry add constraint fk_office_address_id foreign key (office_address_id)
  references address (address_id) match simple
  on update no action on delete set null;

alter table address_book_entry add constraint fk_branch_address_id foreign key (branch_address_id)
  references address (address_id) match simple
  on update no action on delete set null;

alter table branch add constraint fk_address_id foreign key (address_id)
  references address (address_id) match simple
  on update no action on delete no action;

-- sequences

create sequence group_weight_seq;
create sequence dialing_rule_seq;
create sequence ring_seq;
create sequence daily_backup_schedule_seq;
create sequence gateway_seq;
create sequence park_orbit_seq;
create sequence auto_attendant_seq;
create sequence storage_seq;
create sequence dial_plan_seq;
create sequence phone_seq;
create sequence user_seq;
create sequence line_seq;
create sequence job_seq;
create sequence call_group_seq;
create sequence backup_plan_seq;
create sequence meetme_seq;
create sequence extension_pool_seq;
create sequence upload_seq;
create sequence intercom_seq;
create sequence domain_seq;
create sequence phonebook_seq;
create sequence speeddial_seq;
create sequence acd_seq;
create sequence service_seq;
create sequence fxo_port_seq;
create sequence sbc_seq;
create sequence schedule_seq;
create sequence personal_attendant_seq;
create sequence localization_seq;
create sequence paging_group_seq;
create sequence location_seq;
create sequence sipx_service_seq;
create sequence ftp_configuration_seq;
create sequence alarm_server_seq;
create sequence location_specific_service_seq;
create sequence timezone_seq;
create sequence speeddial_group_seq;
create sequence abe_seq;
create sequence phonebook_file_entry_seq;
create sequence branch_seq;
create sequence private_user_key_seq;
create sequence alarm_group_seq;
create sequence alarm_code_seq;
create sequence tls_peer_seq;
create sequence google_domain_seq;
create sequence general_phonebook_settings_seq;
create sequence addr_seq;

-- used for native hibernate ids  
create sequence hibernate_sequence;

-- index
create index index_acd_agent_user_server on acd_agent(acd_agent_id, user_id, acd_server_id);

-- insert standard set of services
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxProxyService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxRegistrarService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxSupervisorService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxParkService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxPresenceService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxIvrService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxRlsService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxStatusService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxCallResolverService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxPageService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxConfigService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxConfigAgentService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxAcdService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxBridgeService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxFreeswitchService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxRelayService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxMrtgService');
insert into sipx_service (sipx_service_id, bean_id) values (nextval('sipx_service_seq'), 'sipxSaaService');

-- insert default values
insert into attendant_special_mode values (1, false, null);
insert into alarm_group (alarm_group_id, name, description, enabled) values (nextval('alarm_group_seq'), 'default', 'Default alarm group', true);
insert into google_domain values (nextval('google_domain_seq'), 'gmail.com');

/* will trigger app event to execute java code before next startup to insert default data */
insert into initialization_task (name) values ('dial-plans');
insert into initialization_task (name) values ('default-phone-group');
insert into initialization_task (name) values ('add_default_user_group');
insert into initialization_task (name) values ('operator');
insert into initialization_task (name) values ('afterhour');
insert into initialization_task (name) values ('first-run');
insert into initialization_task (name) values ('callgroup-password-init');
insert into initialization_task (name) values ('sbc_address_migrate_sbc_device');
insert into initialization_task (name) values ('sip_trunk_address_migrate_sbc_device');
insert into initialization_task (name) values ('migrate_locations');
insert into initialization_task (name) values ('initialize-location-service-mapping');
insert into initialization_task (name) values ('acd_server_migrate_acd_service');
insert into initialization_task (name) values ('default-time-zone');
insert into initialization_task (name) values ('migrate-conference-bridges');
insert into initialization_task (name) values ('phonebook_file_entry_task');
insert into initialization_task (name) values ('mailbox_prefs_migration');
insert into initialization_task (name) values ('legacy_park_server_migration');
insert into initialization_task (name) values ('phonebook_entries_update_task');

