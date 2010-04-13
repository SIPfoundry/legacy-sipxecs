create table discovered_devices (
	mac_address varchar(255) not null,
	ip_address varchar(255) not null,
	vendor varchar(255) not null,
	last_seen timestamp default now(),
	primary key (mac_address)
);
