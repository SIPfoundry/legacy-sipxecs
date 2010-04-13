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

public class SimpleCdrServiceProvider implements CdrServiceProvider {

    public CdrService getCdrService(URL url) throws ServiceException {
        return new CdrImplServiceLocator().getCdrService(url);
    }
}
