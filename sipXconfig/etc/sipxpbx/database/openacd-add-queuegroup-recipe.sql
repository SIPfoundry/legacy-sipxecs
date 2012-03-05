alter table openacd_recipe_step add column openacd_queue_group_id int;
alter table openacd_recipe_step add constraint fk_openacd_queue_group foreign key (openacd_queue_group_id)
  references openacd_queue_group (openacd_queue_group_id) match simple;
alter table openacd_recipe_step alter column openacd_queue_id drop not null;