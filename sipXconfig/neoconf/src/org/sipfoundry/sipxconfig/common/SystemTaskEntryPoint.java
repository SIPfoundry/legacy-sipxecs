/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;


/**
 * For database tasks typically initiated from command line, this will
 * get system started
 */
public interface SystemTaskEntryPoint {

    /**
     * First argument is the beanId, remaining args where passed in from
     * database.xml file
     */
    public void runSystemTask(String[] args);
}
