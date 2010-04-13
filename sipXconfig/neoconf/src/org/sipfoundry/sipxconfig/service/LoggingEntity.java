/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.service;

public interface LoggingEntity {

    /**
     * Implement this method in order to set the log level however it suits your needs.
     */
    void setLogLevel(String logLevel);

    /**
     * Implement this method in order to get the log level however it suits your needs.
     */
    String getLogLevel();

    /**
     * Implement this method in order to retrieve a label key needed in the UI for this logging
     * entity.
     */
    String getLabelKey();
}
