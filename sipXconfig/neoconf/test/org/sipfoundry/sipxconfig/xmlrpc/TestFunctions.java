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
