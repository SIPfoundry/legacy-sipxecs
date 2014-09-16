/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.connection;

import java.sql.Connection;
import java.sql.DatabaseMetaData;
import java.sql.SQLException;

import org.jivesoftware.database.ConnectionProvider;
import org.jivesoftware.openfire.container.Plugin;
import org.jivesoftware.openfire.provider.ConnectionManagerWrapper;

/**
 * Wraps access to mongodb-enabled storage
 *
 * @see ConnectionManagerWrapper
 * @author Alex Mateescu
 *
 */
public class MongoConnMgrWrapper implements ConnectionManagerWrapper {

    private static boolean profilingEnabled;
    private static ConnectionProvider provider;
    private static final Object PROVIDER_LOCK = new Object();

    /**
     * {@inheritDoc} <br/>
     * Newer versions of openuc provide an up-to-date schema. Nothing to do here.
     */
    @Override
    public boolean checkPluginSchema(Plugin plugin) {
        return true;
    }

    /**
     * {@inheritDoc} <br/>
     */
    @Override
    public DatabaseMetaData getMetaData() throws SQLException {
        return new MongoMetaData();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getTestQuery(String driver) {
        return "";
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getTransactionIsolation() {
        return Connection.TRANSACTION_NONE;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isEmbeddedDB() {
        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isProfilingEnabled() {
        return profilingEnabled;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isSetupMode() {
        return provider == null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setConnectionProvider(ConnectionProvider provider) {
        synchronized (new byte[0]) {
            MongoConnMgrWrapper.provider = provider;
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ConnectionProvider getConnectionProvider() {
        return provider;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setProfilingEnabled(boolean enabled) {
        synchronized (new byte[0]) {
            profilingEnabled = enabled;
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void shutdown() {
        synchronized (PROVIDER_LOCK) {
            if (provider != null) {
                provider.destroy();
                provider = null;
            }
        }
    }

}
