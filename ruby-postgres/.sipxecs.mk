ruby-postgres_SRPM = ruby-postgres-0.7.1-2.src.rpm
ruby-postgres_SPEC = $(SRC)/$(PROJ)/ruby-postgres.spec
ruby-postgres_SOURCES = $(SRC)/$(PROJ)/ruby-postgres-0.7.1.tar.gz

# targets not defined, nothing to do
ruby-postgres.autoreconf ruby-postgres.configure ruby-postgres.dist:;

