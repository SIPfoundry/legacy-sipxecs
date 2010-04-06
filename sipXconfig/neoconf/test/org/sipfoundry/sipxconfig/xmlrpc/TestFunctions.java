/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import java.util.Map;

/**
 * Interface that we need to proxy
 */
interface TestFunctions {
    String multiplyTest(String test, int times);
    int calculateTest(String[] names);
    Map create(Map param);
}
