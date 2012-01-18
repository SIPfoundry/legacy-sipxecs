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

import static org.easymock.classextension.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.reportMatcher;
import static org.easymock.classextension.EasyMock.verify;
import static org.easymock.classextension.EasyMock.createMock;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.internal.matchers.InstanceOf;
import org.restlet.data.MediaType;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.callgroup.AbstractRing.Type;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.forwarding.Ring;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.proxy.ProxySettings;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ForwardingResourceTest extends TestCase {
    private static final String CALL_FWD_TIMER_SETTING = "callfwd/timer";
    private User m_user;
    private ForwardingContext m_forwardingContext;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUserName("abc");        

        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        
        ProxyManager proxyManager = createMock(ProxyManager.class);
        ProxySettings settings = new ProxySettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        proxyManager.getSettings();
        expectLastCall().andReturn(settings).anyTimes();
        replay(proxyManager);
        
        m_user.setProxyManager(proxyManager);
        m_user.setPermissionManager(pManager);
        m_user.setSettingTypedValue(CALL_FWD_TIMER_SETTING, 27);

        CallSequence sequence = new CallSequence();
        sequence.setUser(m_user);

        List rings = new ArrayList(5);
        for (int i = 0; i < 5; i++) {
            Ring ring = sequence.insertRing();
            ring.setExpiration(25 + i);
            ring.setType(i % 2 == 0 ? Type.DELAYED : Type.IMMEDIATE);
            ring.setNumber("20" + i);
            ring.setEnabled(true);
            rings.add(ring);
        }

        m_forwardingContext = createMock(ForwardingContext.class);
        m_forwardingContext.getCallSequenceForUser(null);
        expectLastCall().andReturn(sequence);

        replay(m_forwardingContext);
    }

    public void testRepresentXmlNoVoicemail() throws Exception {
        m_user.setVoicemailPermission(false);
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJsonNoVoicemail() throws Exception {
        m_user.setVoicemailPermission(false);
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.rest.test.json"));
        assertEquals(expected, generated);
    }

    public void testRepresentXmlWithVoicemail() throws Exception {
        m_user.setVoicemailPermission(true);
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.restWithVoicemail.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJsonWithVoicemail() throws Exception {
        m_user.setVoicemailPermission(true);
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.restWithVoicemail.test.json"));
        assertEquals(expected, generated);
    }

    public void testStoreXml() throws Exception {
        CallSequence oldCallSequence = new CallSequence();
        oldCallSequence.setUser(m_user);

        CallSequenceMatcher matcher = new CallSequenceMatcher();

        ForwardingContext forwardingContext = createMock(ForwardingContext.class);
        forwardingContext.getCallSequenceForUser(null);
        expectLastCall().andReturn(oldCallSequence);
        forwardingContext.saveCallSequence(callSequence(matcher));
        replay(forwardingContext);

        final InputStream xmlStream = getClass().getResourceAsStream("call-sequence.restToStore.test.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);

        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(forwardingContext);
        resource.storeRepresentation(entity);

        CallSequence savedCallSequence = matcher.getArgument();
        User savedUser = savedCallSequence.getUser();
        assertEquals(m_user, savedUser);
        assertEquals(27, savedUser.getSettingTypedValue(CALL_FWD_TIMER_SETTING));
        assertEquals(5, savedCallSequence.getRings().size());

        for (int i = 0; i < 5; i++) {
            AbstractRing ring = savedCallSequence.getRings().get(i);
            assertEquals(25 + i, ring.getExpiration());
            if (i % 2 == 0) {
                assertEquals(Type.DELAYED, ring.getType());
            } else {
                assertEquals(Type.IMMEDIATE, ring.getType());
            }
        }

        verify(forwardingContext);
    }

    public void testStoreJson() throws Exception {
        CallSequence oldCallSequence = new CallSequence();
        oldCallSequence.setUser(m_user);

        CallSequenceMatcher matcher = new CallSequenceMatcher();

        ForwardingContext forwardingContext = createMock(ForwardingContext.class);
        forwardingContext.getCallSequenceForUser(null);
        expectLastCall().andReturn(oldCallSequence);
        forwardingContext.saveCallSequence(callSequence(matcher));
        replay(forwardingContext);

        final InputStream xmlStream = getClass().getResourceAsStream("call-sequence.restToStore.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);

        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(forwardingContext);
        resource.storeRepresentation(entity);

        CallSequence savedCallSequence = matcher.getArgument();
        User savedUser = savedCallSequence.getUser();
        assertEquals(27, savedUser.getSettingTypedValue(CALL_FWD_TIMER_SETTING));
        assertEquals(m_user, savedUser);
        assertEquals(5, savedCallSequence.getRings().size());

        for (int i = 0; i < 5; i++) {
            AbstractRing ring = savedCallSequence.getRings().get(i);
            assertEquals(25 + i, ring.getExpiration());
            if (i % 2 == 0) {
                assertEquals(Type.DELAYED, ring.getType());
            } else {
                assertEquals(Type.IMMEDIATE, ring.getType());
            }
        }

        verify(forwardingContext);
    }

    private static CallSequence callSequence(CallSequenceMatcher matcher) {
        reportMatcher(matcher);
        return null;
    }

    private static class CallSequenceMatcher extends InstanceOf {
        private CallSequence m_argument;

        public CallSequenceMatcher() {
            super(CallSequence.class);
        }

        @Override
        public boolean matches(Object argument) {
            super.matches(argument);
            m_argument = (CallSequence) argument;
            return true;
        }

        public CallSequence getArgument() {
            return m_argument;
        }
    }
}
