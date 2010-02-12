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

import static org.easymock.EasyMock.expectLastCall;
import static org.restlet.data.MediaType.TEXT_XML;
import static org.restlet.data.MediaType.TEXT_PLAIN;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.channels.ReadableByteChannel;
import java.nio.channels.WritableByteChannel;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.easymock.classextension.EasyMock;
import org.restlet.data.Request;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class ActiveGreetingResourceTest extends TestCase {
    private User m_user;
    private CoreContext m_coreContext;
    private PermissionManager m_pManager;
    private ActiveGreetingResource m_resource;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUserName("200");

        Setting s = new SettingSet();
        s.setName("voicemail/mailbox/active-greeting");
        Setting s1 = new SettingSet("voicemail");
        Setting s2 = new SettingSet("mailbox");
        Setting s3 = new SettingImpl("active-greeting");
        s3.setValue("outofoffice");

        s.addSetting(s1);
        s1.addSetting(s2);
        s2.addSetting(s3);
        m_pManager = EasyMock.createMock(PermissionManager.class);
        m_pManager.getPermissionModel();
        expectLastCall().andReturn(s).anyTimes();
        EasyMock.replay(m_pManager);
        m_user.setPermissionManager(m_pManager);

        m_resource = new ActiveGreetingResource();

        m_coreContext = EasyMock.createMock(CoreContext.class);
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        m_coreContext.saveUser(m_user);
        expectLastCall().andReturn(true).anyTimes();
        m_coreContext.loadUserByUserName(m_user.getUserName());
        expectLastCall().andReturn(m_user);
        EasyMock.replay(m_coreContext);
        m_resource.setCoreContext(m_coreContext);
    }

    public void testRepresentXml() throws Exception {
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("user", "200");
        request.setAttributes(attributes);
        m_resource.setRequest(request);
        m_resource.init(null, request, null);

        Representation representation = m_resource.represent(new Variant(TEXT_XML));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String greeting = writer.toString();
        String expectedXml = IOUtils.toString(getClass().getResourceAsStream("expected-activegreeting.xml"));
        assertEquals(expectedXml, greeting);
    }

    public void testRepresent() throws Exception {
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("user", "200");
        request.setAttributes(attributes);
        m_resource.setRequest(request);
        m_resource.init(null, request, null);

        Representation representation = m_resource.represent(new Variant(TEXT_PLAIN));
        StringWriter writer = new StringWriter();
        representation.write(writer);
        String greeting = writer.toString();
        assertEquals("outofoffice", greeting);
    }

    public void testStoreRepresentation() throws Exception {
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("user", "200");
        attributes.put("greeting", "extendedabsence");
        request.setAttributes(attributes);
        m_resource.setRequest(request);
        m_resource.init(null, request, null);

        Representation representation = new Representation() {
            @Override
            public void write(Writer arg0) throws IOException {
            }

            @Override
            public void write(WritableByteChannel arg0) throws IOException {
            }

            @Override
            public void write(OutputStream arg0) throws IOException {
            }

            @Override
            public InputStream getStream() throws IOException {
                return null;
            }

            @Override
            public Reader getReader() throws IOException {
                return null;
            }

            @Override
            public ReadableByteChannel getChannel() throws IOException {
                return null;
            }
        };
        m_resource.storeRepresentation(representation);
        assertEquals("extendedabsence", m_coreContext.loadUserByUserName("200").getSettingValue(
                "voicemail/mailbox/active-greeting"));
    }

    public void testStoreRepresentationBadValue() throws Exception {
        Request request = new Request();
        Map<String, Object> attributes = new HashMap<String, Object>();
        attributes.put("user", "200");
        attributes.put("greeting", "blablabla");
        request.setAttributes(attributes);
        m_resource.setRequest(request);
        m_resource.init(null, request, null);

        Representation representation = new Representation() {

            @Override
            public void write(Writer arg0) throws IOException {
            }

            @Override
            public void write(WritableByteChannel arg0) throws IOException {
            }

            @Override
            public void write(OutputStream arg0) throws IOException {
            }

            @Override
            public InputStream getStream() throws IOException {
                return null;
            }

            @Override
            public Reader getReader() throws IOException {
                return null;
            }

            @Override
            public ReadableByteChannel getChannel() throws IOException {
                return null;
            }
        };
        m_resource.storeRepresentation(representation);
        assertEquals("none", m_coreContext.loadUserByUserName("200").getSettingValue(
                "voicemail/mailbox/active-greeting"));
    }

}
