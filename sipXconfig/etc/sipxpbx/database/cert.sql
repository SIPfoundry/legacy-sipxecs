create table authority (
   name varchar(255) not null unique,
   data text not null,
   private_key text,
   primary key (name)
);

create table cert (
   name varchar(255) not null unique,      
   data text not null,
   private_key text,
   authority varchar(255),
   primary key (name)
);
