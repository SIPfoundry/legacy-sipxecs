create table attendant_special_mode (
    attendant_special_mode_id int4 not null,
    enabled boolean not null default false,
    auto_attendant_id int4,
    primary key (attendant_special_mode_id)
);

insert into attendant_special_mode values (1, false, null);
