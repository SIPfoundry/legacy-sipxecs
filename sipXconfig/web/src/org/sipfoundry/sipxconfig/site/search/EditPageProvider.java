/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.search;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;

/**
 * Strategy for locating pages based on object class and id.
 */
public interface EditPageProvider {
    IPage getPage(IRequestCycle cycle, Class klass, Object id);
}
