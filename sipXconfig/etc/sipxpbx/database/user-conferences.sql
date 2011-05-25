alter table meetme_conference ADD owner_id int4;
alter table meetme_conference add constraint fk_owner_id foreign key (owner_id)
    references users(user_id) on delete set null;