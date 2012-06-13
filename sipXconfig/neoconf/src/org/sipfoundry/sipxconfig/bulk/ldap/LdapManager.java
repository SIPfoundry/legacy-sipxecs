/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.List;

import javax.naming.NamingException;
import javax.naming.directory.DirContext;

import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.springframework.ldap.DirContextProcessor;

public interface LdapManager {
    public static final String FILTER_ALL_CLASSES = "objectclass=*";
    public static final String CONTEXT_BEAN_NAME = "ldapManager";

    public static final DirContextProcessor NULL_PROCESSOR = new DirContextProcessor() {
        public void postProcess(DirContext ctx) throws NamingException {
        }
        public void preProcess(DirContext ctx) throws NamingException {
        }
    };

    /**
     * Retrieves LDAP schema.
     *
     * Schema contains list of object classes and their attributes.
     *
     * @throws UserException if connection is not possible for some reason
     */
    Schema getSchema(String subschemaSubentry, LdapConnectionParams params);

    LdapConnectionParams getConnectionParams(int connectionId);

    void setConnectionParams(LdapConnectionParams params);

    AttrMap getAttrMap(int connectionId);

    void setAttrMap(AttrMap attrMap);

    CronSchedule getSchedule(int connectionId);

    void setSchedule(CronSchedule schedule, int connectionId);

    /**
     * Check LDAP connection for the provided connection params
     *
     * @throws UserException if connection is not possible for some reason
     */
    void verify(LdapConnectionParams params, AttrMap attrMap);

    boolean verifyLdapConnection(LdapConnectionParams params);

    void saveSystemSettings(LdapSystemSettings settings);

    LdapSystemSettings getSystemSettings();

    void replicateOpenfireConfig();

    public List<LdapConnectionParams> getAllConnectionParams();

    public boolean verifyAllLdapConnections();

    public LdapConnectionParams createConnectionParams();

    public void removeConnectionParams(int connectionId);

    public AttrMap createAttrMap();
}
