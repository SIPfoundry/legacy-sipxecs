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

alter table site_to_site_dialing_rule
    add constraint site_to_site_dialing_rule_dialing_rule
    foreign key (site_to_site_dialing_rule_id)
    references dialing_rule;

alter table site_to_site_dial_pattern
    add constraint site_to_site_dialing_pattern_dialing_rule
    foreign key (site_to_site_dialing_rule_id)
    references site_to_site_dialing_rule;
