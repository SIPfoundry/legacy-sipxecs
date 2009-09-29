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

import java.util.List;


public class ExtensionPoolContextImpl extends SipxHibernateDaoSupport implements ExtensionPoolContext {

    /** Name of the user extension pool */
    public static final String USER_POOL_NAME = "user";

    /** Name of the name property */
    private static final String PROP_NAME = "name";

    // Default pool values.  When we add multiple pools later, we'll need to have
    // different defaults for each named pool.
    private static final boolean DEFAULT_ENABLED = true;
    private static final Integer DEFAULT_FIRST_EXTENSION = new Integer(200);
    private static final Integer DEFAULT_LAST_EXTENSION = new Integer(299);
    private static final Integer DEFAULT_NEXT_EXTENSION = new Integer(200);

    private CoreContext m_coreContext;

    public ExtensionPool getUserExtensionPool() {
        return getExtensionPool(USER_POOL_NAME);
    }

    public void saveExtensionPool(ExtensionPool pool) {
        getHibernateTemplate().saveOrUpdate(pool);
    }

    /**
     * Return the next free extension from the user extension pool,
     * or null if a free extension could not be found.
     */
    public Integer getNextFreeUserExtension() {
        Integer ext = null;     // return value
        ExtensionPool pool = getUserExtensionPool();

        // If the pool is disabled or has no firstExtension, then bail out
        if (!pool.isEnabled() || pool.getFirstExtension() == null) {
            return null;
        }
        int firstExt = pool.getFirstExtension().intValue();

        // Start the search at the pool's nextExtension if that's defined,
        // otherwise the pool's firstExtension
        int start = pool.getNextExtension() != null ? pool.getNextExtension().intValue()
                                                    : firstExt;

        // End the search at the pool's lastExtension
        int end = pool.getLastExtension() != null ? pool.getLastExtension().intValue()
                                                  : Integer.MAX_VALUE;

        // Look for free extensions, starting with the desired next extension
        ext = getFreeUserExtension(start, end);

        // If that didn't work, then try searching the beginning of the range
        if (ext == null && firstExt < start) {
            ext = getFreeUserExtension(firstExt, start - 1);
        }

        return ext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /** Return the named extension pool.  Create it if necessary. */
    private ExtensionPool getExtensionPool(String poolName) {
        List pools = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "extensionPoolByName", PROP_NAME, poolName);
        ExtensionPool pool = null;

        // Create the pool if it doesn't exist
        if (SipxCollectionUtils.safeIsEmpty(pools)) {
            // Create the pool
            pool = new ExtensionPool(DEFAULT_ENABLED, poolName, DEFAULT_FIRST_EXTENSION,
                    DEFAULT_LAST_EXTENSION, DEFAULT_NEXT_EXTENSION);
            saveExtensionPool(pool);
        } else {
            // Return the existing pool.
            // Don't worry about the case of more than one pool with this name existing,
            // there is a DB constraint to prevent that from happening.
            pool = (ExtensionPool) pools.get(0);
        }
        return pool;
    }

    /**
     * Look for the next free user extension in the specified range.
     * The caller is responsible for ensuring that "start" and "end" lie within
     * the pool range.
     */
    // TODO (XCF-583): search for free extensions in a much more efficient way
    private Integer getFreeUserExtension(int start, int end) {
        Integer ext = null;   // the result
        for (int i = start; i <= end; i++) {
            if (m_coreContext.loadUserByUserNameOrAlias(Integer.toString(i)) == null) {
                // No user has that extension, so use it
                ext = new Integer(i);
                break;
            }
        }
        return ext;
    }

}
