create table call_rate_rule (
   call_rate_rule_id int4 not null primary key,
   start_ip varchar(255) not null,
   end_ip varchar(255),
   name varchar(255) not null,
   position int4,
   description varchar(255)
);

create sequence call_rate_rule_seq;

create table call_rate_limit (
   call_rate_limit_id int4 not null,
   call_rate_rule_id int4 not null,   
   rate integer not null,
   sip_method varchar(255) not null,
   interval varchar(255) not null,
   constraint pk_call_rate_limit_id_call_rate_rule_id primary key (call_rate_limit_id, call_rate_rule_id),
   constraint fk_call_rate_limit_call_rate_rule foreign key (call_rate_rule_id)
      references call_rate_rule (call_rate_rule_id) match full
      on update no action on delete no action
);

create sequence call_rate_limit_seq;
