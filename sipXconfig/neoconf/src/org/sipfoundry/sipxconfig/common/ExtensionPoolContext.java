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

public interface ExtensionPoolContext extends DataObjectSource {
    public static final String CONTEXT_BEAN_NAME = "extensionPoolContext";

    /** Return the user extension pool.  Create it if necessary. */
    public ExtensionPool getUserExtensionPool();

    public void saveExtensionPool(ExtensionPool pool);

    /**
     * Return the next free extension from the user extension pool,
     * or null if a free extension could not be found.
     */
    public Integer getNextFreeUserExtension();
}
