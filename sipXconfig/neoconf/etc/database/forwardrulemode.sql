/*
 * As part of XCF-858 modify ring table add enabled column and set default to true
 */
 
alter table ring add column enabled bool;
update ring set enabled = true;
alter table ring alter column enabled set not null;
alter table ring alter column enabled set default true;
    