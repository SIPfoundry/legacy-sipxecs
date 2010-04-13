/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.ValidatorException;
import org.easymock.EasyMock;

import junit.framework.TestCase;

public class DomainAliasValidatorTest extends TestCase {

    IFormComponent m_field;
    IPage m_page;
    IComponent m_component;
    IBinding m_binding;

    @Override
    public void setUp() {
        m_page = EasyMock.createMock(IPage.class);
        m_field = EasyMock.createMock(IFormComponent.class);
        m_component = EasyMock.createMock(IComponent.class);
        m_binding = EasyMock.createMock(IBinding.class);

        final class MyMessages implements Messages {
            @Override
            public String format(String arg0, Object[] arg1) {
                return null;
            }

            @Override
            public String format(String arg0, Object arg1) {
                return null;
            }

            @Override
            public String format(String arg0, Object arg1, Object arg2) {
                return null;
            }

            @Override
            public String format(String arg0, Object arg1, Object arg2, Object arg3) {
                return null;
            }

            @Override
            public String getMessage(String arg0) {
                return "";
            }
        }

        m_field.getPage();
        EasyMock.expectLastCall().andReturn(m_page).anyTimes();
        m_page.getMessages();
        EasyMock.expectLastCall().andReturn(new MyMessages()).anyTimes();

        m_page.getComponent("name");
        EasyMock.expectLastCall().andReturn(m_component).anyTimes();

        m_component.getBinding("value");
        EasyMock.expectLastCall().andReturn(m_binding).anyTimes();
    }

    public void testInvalid() throws Exception {
        DomainAliasValidator out = new DomainAliasValidator();
        try {
            m_binding.getObject();
            EasyMock.expectLastCall().andReturn("mydomain.example.org").anyTimes();

            EasyMock.replay(m_field, m_page, m_component, m_binding);
            out.validate(m_field, null, "mydomain.example.org");
        } catch (ValidatorException e) {
            return;
        }
        fail();
    }

    public void testValid() throws Exception {
        DomainAliasValidator out = new DomainAliasValidator();
        try {
            m_binding.getObject();
            EasyMock.expectLastCall().andReturn("11.126.12.20").anyTimes();

            EasyMock.replay(m_field, m_page, m_component, m_binding);
            out.validate(m_field, null, "mydomain.example.org");
        } catch (ValidatorException e) {
            fail();
        }
        return;
    }

}
