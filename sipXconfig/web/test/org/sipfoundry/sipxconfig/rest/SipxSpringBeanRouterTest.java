/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.rest;

import java.util.ArrayList;
import java.util.List;

import org.restlet.ext.spring.SpringBeanFinder;

import junit.framework.TestCase;
import org.restlet.Restlet;
import org.restlet.Route;

public class SipxSpringBeanRouterTest extends TestCase {

    public void testAttachFinder() {
        final List<String> uris = new ArrayList<String>();
        final SpringBeanFinder finder = new SpringBeanFinder();

        SipxSpringBeanRouter router = new SipxSpringBeanRouter() {
            @Override
            public Route attach(String uriPattern, Restlet target) {
                assertSame(finder, target);
                uris.add(uriPattern);
                return null;
            }
        };

        // adds one finder
        router.attachFinder("/some/uri", finder);

        // adds 2 finders
        router.attachFinder("/my/some/uri", finder);

        // adds 3 finders
        router.attachFinder("/my/call/some/uri", finder);

        assertEquals(6, uris.size());
        assertEquals("/some/uri", uris.get(0));
        assertEquals("/my/some/uri", uris.get(1));
        assertEquals("/private/{puk}/some/uri", uris.get(2));
        assertEquals("/my/call/some/uri", uris.get(3));
        assertEquals("/private/{puk}/call/some/uri", uris.get(4));
        assertEquals("/call/some/uri", uris.get(5));
    }
}
