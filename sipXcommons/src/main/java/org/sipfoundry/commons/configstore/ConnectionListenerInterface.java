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
