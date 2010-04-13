/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.configdb;

import java.util.Hashtable;

/**
 * Interface to OsConfigDb to be called through XML/RPC
 *
 * See: https://scm.sipfoundry.org/rep/sipX/main/sipXcommserverLib/src/configrpc/README
 */
public interface ConfigDbParameter {
    public static final String METHOD_NAME_PREFIX = "configurationParameter.";

    /**
     * Sets each param-name => param-value pair in dbName.
     *
     * Either all sets are made or none are made.
     *
     * @param dbName name of the database
     * @param params collection of param-name => param-value pairs
     * @return numver of parameters set
     */
    int set(String dbName, Hashtable params);

    /**
     * Returns a struct containing the name and value for each parameter in the input array of
     * parameter names.
     *
     * If any parameter in the set is undefined, a PARAMETER_UNDEFINED fault is returned. *
     *
     * @param dbName name of the database
     * @param paramNames list of parameter names
     * @return collection of param-name => param-value pairs
     */
    Hashtable get(String dbName, String[] paramNames);

    /**
     * Returns a struct containing the name and value of all the parameters in the database.
     *
     * When called with just the db_name, if the dataset is empty (there are no parameters
     * defined), a DATASET_EMPTY fault is returned.
     *
     * @param dbName name of the database
     * @return collection of param-name => param-value pairs
     */
    Hashtable get(String dbName);

    /**
     *
     * @param dbName name of the database
     * @param paramNames list of parameter names
     * @return number of parameters deleted
     */
    int delete(String dbName, String[] paramNames);

    /**
     * This method should be called by a configuration application to confirm that it is using a
     * compatible definition of the database definition. If the returned version_id does not match
     * what the configuration application expects, it should not modify the configuration..
     *
     * @param dbName name of the database
     * @return version identifier
     */
    String version(String dbName);
}
