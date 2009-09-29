/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.collections.Predicate;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

public class FilteredModelSourceTest extends TestCase {

    private List<PhoneModel> m_models;
    private FilteredModelSource m_modelSource;
    private ModelSource m_mockSource;

    protected void setUp() throws Exception {
        m_models = Arrays.asList(new PhoneModel[] {
            new PhoneModel("beanId", "falcon300"),
            new PhoneModel("beanId", "vulture7960"),
            new PhoneModel("beanId", "sparrow420"),
            new PhoneModel("beanId", "kestrel"),
            new PhoneModel("beanId", "kestrel1000")
        });
        m_mockSource = EasyMock.createMock(ModelSource.class);
        m_mockSource.getModels();
        EasyMock.expectLastCall().andReturn(m_models).anyTimes();
        EasyMock.replay(m_mockSource);

        m_modelSource = new FilteredModelSource();
        m_modelSource.setModelSource(m_mockSource);
    }

    protected void tearDown() throws Exception {
        EasyMock.verify(m_mockSource);
    }

    public void testGetAvailablePhoneModels() {
        assertSame(m_models, m_modelSource.getModels());
    }

    public void testGetCertifiedPhones() {
        m_modelSource.setCertified("^(falcon.*)$");
        Collection certified = m_modelSource.getModels();
        assertEquals(1, certified.size());
        assertSame(m_models.get(0), certified.iterator().next());
    }

    public void testRemoveUnCertifiedPhones() {
        m_modelSource.setCertified("^(?!(vulture|sparrow)).*$");
        Collection certified = m_modelSource.getModels();
        assertEquals(3, certified.size());
        assertTrue(certified.contains(m_models.get(0)));
        assertFalse(certified.contains(m_models.get(1)));
        assertFalse(certified.contains(m_models.get(2)));
        assertTrue(certified.contains(m_models.get(3)));
        assertTrue(certified.contains(m_models.get(4)));
    }

    public void testSetFilter() {
        Predicate onlySparrow = new Predicate() {
            public boolean evaluate(Object object) {
                return object.equals(m_models.get(0));
            }
        };
        m_modelSource.setFilter(onlySparrow);
        Collection certified = m_modelSource.getModels();
        assertEquals(1, certified.size());
        assertTrue(certified.contains(m_models.get(0)));
    }
}
