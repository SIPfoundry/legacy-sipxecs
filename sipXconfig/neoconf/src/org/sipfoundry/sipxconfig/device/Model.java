/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import org.springframework.beans.factory.BeanNameAware;

/**
 * Marker interface - needs to be implemented by objects used by ModelSorce
 *
 * Implementing BeanNameAware ensures that models have access to their bean IDs
 */
public interface Model extends BeanNameAware {
}
