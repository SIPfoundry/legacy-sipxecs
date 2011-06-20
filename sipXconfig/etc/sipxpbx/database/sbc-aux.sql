alter table sbc add column sbc_type char(1);

update sbc set sbc_type = 'D';
alter table sbc alter column sbc_type set not null;
