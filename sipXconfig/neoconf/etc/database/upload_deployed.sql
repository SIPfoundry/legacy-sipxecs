/* 
 *  enabled bool not null
 */
alter table upload add column deployed bool;
update upload set deployed = true;
alter table upload alter column deployed set not null;

