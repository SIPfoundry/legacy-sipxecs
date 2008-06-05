/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

public interface NatTraversalManager {
    public static final String CONTEXT_BEAN_NAME = "natTraversalManager";

    public void store(NatTraversal natTraversal);
    public NatTraversal getNatTraversal();
}
