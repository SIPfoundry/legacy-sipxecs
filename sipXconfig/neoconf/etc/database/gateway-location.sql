alter table gateway add column shared boolean;
update gateway set shared=false;
alter table gateway alter column shared set not null;

alter table gateway add column site_id int4;

alter table gateway add constraint fk_gateway_site
    foreign key (site_id)
    references group_storage;
