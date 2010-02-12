package org.sipfoundry.commons.configstore;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.LinkedList;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ConfigStore {
    private final String host;
    private final int port;
    private final String database;
    private final String user;
    private final String password;
    private HashMap<String, PersistedCollection<? extends PersistedClass>> persistedCollections;
    private PostgresqlConnector dbConnector;
    private LinkedList<String> tableList;

    public ConfigStore(String host, int port, String database, String user, String password) {
        this.host = host;
        this.port = port;
        this.database = database;
        this.user = user;
        this.password = password;
        
        persistedCollections = new HashMap<String, PersistedCollection<? extends PersistedClass>>();
        tableList = new LinkedList<String>();
    }
    
    public void addCollection(PersistedCollection<? extends PersistedClass> collection) {
        persistedCollections.put(collection.getDbTable(), collection);
        tableList.add(collection.getDbTable());
    }
    
    public void start() {
        try {
            dbConnector = new PostgresqlConnector(this, host, port, database, user, password);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }
    
    public void stop() {
        dbConnector.stop();
    }
    
    protected LinkedList<String> getTableList() {
        return tableList;
    }
    
    protected String getPrimaryDbKey(String table) {
        String primaryDbKey = null;
        
        PersistedCollection<? extends PersistedClass> collection = persistedCollections.get(table);
        if (collection != null) {
            primaryDbKey = collection.getPrimaryDbKey();
        }
        
        return primaryDbKey;
    }
    
    protected void processRowUpdate(String table, ResultSet rs, ConnectionListenerInterface connectionListener) throws SQLException {
        PersistedCollection<? extends PersistedClass> collection = persistedCollections.get(table);
        if (collection != null) {
            collection.processRowUpdate(rs, connectionListener);
        }
    }
    
    protected void processColumnUpdate(String table, ResultSet rs, ConnectionListenerInterface connectionListener) throws SQLException {
        PersistedCollection<? extends PersistedClass> collection = persistedCollections.get(table);
        if (collection != null) {
            collection.processColumnUpdate(rs, connectionListener);
        }
    }
}
