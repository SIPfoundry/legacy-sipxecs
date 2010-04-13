/*
 * XCF-819, Grandstream model_ids have changed
 * upgrade to new model_ids and remove all settings
 */

update phone set model_id='PhoneBt' where model_id='BudgeTone';
update phone set model_id='Ht286' where model_id='HandyTone';
delete from setting_value where value_storage_id in
             (select value_storage_id from phone where bean_id='grandstream' and value_storage_id is not null);
