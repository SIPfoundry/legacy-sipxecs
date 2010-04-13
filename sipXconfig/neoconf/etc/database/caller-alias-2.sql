-- no need for external number any more: it's kept in settings
alter table users drop column external_number;

-- additional gateway columns
alter table gateway add column anonymous boolean;
alter table gateway add column ignore_user_info boolean;
alter table gateway add column transform_user_extension boolean;
alter table gateway add column add_prefix varchar(255);
alter table gateway add column keep_digits integer;

-- initialize default values
update gateway set
        anonymous=false,
        ignore_user_info=false,
        transform_user_extension=false,
        keep_digits=0;

alter table gateway alter column anonymous set not null;
alter table gateway alter column ignore_user_info set not null;
alter table gateway alter column transform_user_extension set not null;
alter table gateway alter column keep_digits set not null;
