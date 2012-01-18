create table feature_local (
   feature_id varchar(255) not null,
   location_id int4 not null,
   primary key (location_id, feature_id)
);

create table feature_global (
   feature_id varchar(255) unique not null
);
