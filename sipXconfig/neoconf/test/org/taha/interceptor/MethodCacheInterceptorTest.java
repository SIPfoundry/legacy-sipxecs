/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.taha.interceptor;

import junit.framework.TestCase;

public class MethodCacheInterceptorTest extends TestCase {

    public void testGetCacheKey() {
        assertEquals("a", MethodCacheInterceptor.getCacheKey("a"));
        assertEquals("[a,b,]", MethodCacheInterceptor.getCacheKey(new String[] {"a", "b"}));
        assertEquals("[a,[b,c,],]", MethodCacheInterceptor.getCacheKey(new Object[] {"a",
                new Object[] { "b", "c" }}));
    }
}
