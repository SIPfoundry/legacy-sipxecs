/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.configstore;

import java.sql.ResultSet;
import java.sql.SQLException;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public interface ConnectionListenerInterface {
    public void start();
    public void shutdown();
    public ResultSet retrieveRow(String table, String primaryDbKey, String keyValue) throws SQLException;

}
