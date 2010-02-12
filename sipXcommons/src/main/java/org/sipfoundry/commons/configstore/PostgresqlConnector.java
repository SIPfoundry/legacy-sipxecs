package org.sipfoundry.commons.configstore;

import java.sql.*;
import java.util.HashMap;
import java.util.LinkedList;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class PostgresqlConnector {
    private final ConnectionListener listener;

    public PostgresqlConnector(ConfigStore configStore, String host, int port, String database, String user, String password) throws ClassNotFoundException {
        // Test if the JDBC driver is available. Will throw an exception if not.
        Class.forName("org.postgresql.Driver");

        String url = "jdbc:postgresql://" + host + ":" + port + "/" + database;

        listener = new ConnectionListener(configStore, url, user, password);
        listener.start();
    }

    public void stop() {
        listener.shutdown();
    }
}

class ConnectionListener extends Thread implements ConnectionListenerInterface {
    private final ConfigStore configStore;
    private final String url;
    private final String user;
    private final String password;

    private boolean running;
    private boolean connected;
    private boolean forceReload;
    private LinkedList<String> tableList;
    private HashMap<String, String> notificationTableMap;
    private Connection conn;
    private org.postgresql.PGConnection pgconn;
    private PreparedStatement pingStatement;
    private Statement retrieveKeysStatement;
    private Statement retrieveRowStatement;
    private Statement retrieveAllStatement;

    ConnectionListener(ConfigStore configStore, String url, String user, String password) {
        super("ConfigStore");
        
        this.configStore = configStore;
        this.url = url;
        this.user = user;
        this.password = password;

        running = true;
        connected = false;
        forceReload = false;
        
        notificationTableMap = new HashMap<String, String>();
    }

    public void run() {
        // Build up the notification / table map.
        tableList = configStore.getTableList();
        for (String table : tableList) {
            notificationTableMap.put(table + "_insert", table);
            notificationTableMap.put(table + "_update", table);
            notificationTableMap.put(table + "_delete", table);
        }
        
        while (running) {
            try {
                Thread.sleep(2500);
            } catch (InterruptedException e1) {
            }

            if (!connected) {
                try {
                    conn = DriverManager.getConnection(url, user, password);
                } catch (SQLException e) {
                    // Failed to connect. Go back and try again later.
                    continue;
                }

                pgconn = (org.postgresql.PGConnection) conn;
                try {
                    pingStatement = conn.prepareStatement("SELECT 1");
                    
                    Statement stmt = conn.createStatement();
                    for (String table : tableList) {
                        stmt.execute("LISTEN " + table + "_insert");
                        stmt.execute("LISTEN " + table + "_update");
                        stmt.execute("LISTEN " + table + "_delete");
                    }
                    stmt.close();
                    
                    retrieveKeysStatement = conn.createStatement();
                    retrieveRowStatement = conn.createStatement();
                    retrieveAllStatement = conn.createStatement();
                } catch (SQLException e) {
                    // Fatal
                    e.printStackTrace();
                }

                connected = true;
                forceReload = true;
            }

            try {
                if (forceReload) {
                    ResultSet rs = null;
                    for (String table : tableList) {
                        String primaryDbKey = configStore.getPrimaryDbKey(table);
                        rs = retrieveKeysStatement.executeQuery("SELECT " + primaryDbKey + " FROM " + table);
                        configStore.processRowUpdate(table, rs, this);
                    }
                    rs.close();
                    forceReload = false;
                } else {
                    ResultSet rs = pingStatement.executeQuery();

                    org.postgresql.PGNotification notifications[] = pgconn.getNotifications();
                    if (notifications != null) {
                        for (int i = 0; i < notifications.length; i++) {
                            String notification = notifications[i].getName();
                            String table = notificationTableMap.get(notification);
                            if (notification.endsWith("_update")) {
                                rs = retrieveAllStatement.executeQuery("SELECT * FROM " + table);
                                configStore.processColumnUpdate(table, rs, this);
                            } else {
                                String primaryDbKey = configStore.getPrimaryDbKey(table);
                                rs = retrieveKeysStatement.executeQuery("SELECT " + primaryDbKey + " FROM " + table);
                                configStore.processRowUpdate(table, rs, this);
                            }
                        }
                    }
                    rs.close();
                }
            } catch (SQLException e) {
                // Lost connection.
                connected = false;
                continue;
            }

        }
        // Shutting down.
        try {
            retrieveAllStatement.close();
            retrieveRowStatement.close();
            retrieveKeysStatement.close();
            conn.close();
        } catch (SQLException e) {
            // Ignore.
        }
    }

    public void shutdown() {
        running = false;
    }

    public ResultSet retrieveRow(String table, String primaryDbKey, String keyValue) throws SQLException {
        ResultSet rs = retrieveRowStatement.executeQuery("SELECT * FROM " + table + " WHERE " + primaryDbKey + " = '" + keyValue + "'");
        return rs;
    }
}
