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
    Schema getSchema(String subschemaSubentry);

    LdapConnectionParams getConnectionParams();

    void setConnectionParams(LdapConnectionParams params);

    AttrMap getAttrMap();

    void setAttrMap(AttrMap attrMap);

    CronSchedule getSchedule();

    void setSchedule(CronSchedule schedule);

    /**
     * Check LDAP connection for the provided connection params
     *
     * @throws UserException if connection is not possible for some reason
     */
    void verify(LdapConnectionParams params, AttrMap attrMap);

    boolean verifyLdapConnection();

    void saveSystemSettings(LdapSystemSettings settings);

    LdapSystemSettings getSystemSettings();

    void replicateOpenfireConfig();
}
