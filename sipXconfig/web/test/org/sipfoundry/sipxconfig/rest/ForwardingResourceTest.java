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

import java.io.InputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.easymock.internal.matchers.InstanceOf;
import org.restlet.data.MediaType;
import org.restlet.resource.InputRepresentation;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.reportMatcher;
import static org.easymock.EasyMock.verify;

public class ForwardingResourceTest extends TestCase {
    private User m_user;
    private ForwardingContext m_forwardingContext;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUserName("abc");

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

    public void testRepresentXml() throws Exception {
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(m_forwardingContext);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("call-sequence.rest.test.json"));
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

        final InputStream xmlStream = getClass().getResourceAsStream("call-sequence.rest.test.xml");
        Representation entity = new InputRepresentation(xmlStream, MediaType.TEXT_XML);

        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(forwardingContext);
        resource.storeRepresentation(entity);

        CallSequence savedCallSequence = matcher.getArgument();

        assertEquals(m_user, savedCallSequence.getUser());
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

        final InputStream xmlStream = getClass().getResourceAsStream("call-sequence.rest.test.json");
        Representation entity = new InputRepresentation(xmlStream, MediaType.APPLICATION_JSON);

        ForwardingResource resource = new ForwardingResource();
        resource.setForwardingContext(forwardingContext);
        resource.storeRepresentation(entity);

        CallSequence savedCallSequence = matcher.getArgument();

        assertEquals(m_user, savedCallSequence.getUser());
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
