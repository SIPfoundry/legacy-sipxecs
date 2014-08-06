create table schedule_feature (
   schedule_id int4 not null,
   feature_id varchar(255) not null,
   primary key (schedule_id, feature_id)
);
