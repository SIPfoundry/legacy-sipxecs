/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig;

import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;

public class EasyMockTest extends TestCase {

    public void testBeanWithoutGenerics() {
        IMocksControl simpleCtrl = EasyMock.createControl();
        simpleCtrl.createMock(BeanWithoutGenerics.class);
    }

    public void testBeanWithGenericsDefined() {
        IMocksControl simpleCtrl = EasyMock.createControl();
        simpleCtrl.createMock(BeanWithGenericsDefined.class);
    }

    public void testBeanWithGenericsInMethod() {
        IMocksControl simpleCtrl = EasyMock.createControl();
        simpleCtrl.createMock(BeanWithGenericsInMethod.class);
    }

    public static class BeanWithoutGenerics {
    }

    public static class BeanWithGenericsDefined {
        private Set<String> m_set = new HashSet<String>();
        public BeanWithGenericsDefined() {
            m_set = new HashSet<String>();
        }
        public Object avoidEclipseWarning() {
            return m_set;
        }
    }

    public static class BeanWithGenericsInMethod {
        private Set<String> m_set = new HashSet<String>();
        public BeanWithGenericsInMethod() {
            init();
        }
        protected void init() {
            m_set = new HashSet<String>();
        }
        public Object avoidEclipseWarning() {
            return m_set;
        }
    }
}
