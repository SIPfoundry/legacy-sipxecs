alter table location
  add column use_stun boolean not null,
  add column stun_address varchar(255),
  add column stun_interval integer not null,
  add column public_address varchar(255),
  add column public_port integer not null,
  add column start_rtp_port integer not null,
  add column stop_rtp_port integer not null;

update location
  set
    use_stun=true,
    stun_interval = 60,
    stun_address = 'stun01.sipphone.com',
    public_port=5060,
    start_rtp_port=30000,
    stop_rtp_port=31000;

alter table location
  alter column use_stun set default true,
  alter column stun_interval set default 60,
  alter column public_port set default 5060,
  alter column start_rtp_port set default 30000,
  alter column stop_rtp_port set default 31000;

-- remove NAT traversal table, keep parameters in sipXrelay service model
drop table nat_traversal;
