alter table address_book_entry add column use_branch_address boolean;

update address_book_entry set use_branch_address = true;
alter table address_book_entry alter column use_branch_address set not null;
alter table address_book_entry alter column use_branch_address set default true;