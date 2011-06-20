/*
 * "Call direction" is an application-level plugin. 
 *
 * Call direction is encoded in char(1) as follows: 
 *
 *   Incoming (I): for calls that come in from a PSTN gateway
 *   Outgoing (O): for calls that go out to a PSTN gateway
 *   Intranetwork (A): for calls that are pure SIP and don't go through a gateway
 */

/*
 * Special Users.
 *
 * cdrremote - user id that provides read-only access to the cdrs table only.  Used for
 *             remote CDR database access.
 */
create user  cdrremote NOSUPERUSER NOCREATEDB  NOCREATEROLE;
grant select on cdrs to cdrremote;
grant select on view_cdrs to cdrremote;
