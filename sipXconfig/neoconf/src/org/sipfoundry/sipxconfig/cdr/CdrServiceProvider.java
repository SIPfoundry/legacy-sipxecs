/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.net.URL;

import javax.xml.rpc.ServiceException;

/**
 * An interface to provide a CdrService
 */
public interface CdrServiceProvider {
    CdrService getCdrService(URL url) throws ServiceException;
}
