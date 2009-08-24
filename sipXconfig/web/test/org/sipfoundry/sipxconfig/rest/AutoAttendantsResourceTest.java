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

import java.io.StringWriter;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;

import org.restlet.data.MediaType;
import org.restlet.resource.Representation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class AutoAttendantsResourceTest extends TestCase {
    private AutoAttendantManager m_autoAttendantManager;

    @Override
    protected void setUp() throws Exception {
        AutoAttendant operator = new AutoAttendant();
        operator.setUniqueId();
        operator.setPromptsDirectory("prompts/");
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        operator.setName("operator");
        operator.setPrompt("operator.wav");

        AutoAttendant afterHour = new AutoAttendant();
        afterHour.setUniqueId();
        afterHour.setName("After hours");

        afterHour.setPromptsDirectory("prompts/");
        afterHour.setPrompt("prompt.wav");
        afterHour.setSystemId(AutoAttendant.AFTERHOUR_ID);

        m_autoAttendantManager = createMock(AutoAttendantManager.class);
        m_autoAttendantManager.getAutoAttendants();
        expectLastCall().andReturn(Arrays.asList(operator, afterHour));

        m_autoAttendantManager.getSpecialMode();
        expectLastCall().andReturn(true);

        m_autoAttendantManager.getSpecialMode();
        expectLastCall().andReturn(true);

        m_autoAttendantManager.getSelectedSpecialAttendant();
        expectLastCall().andReturn(afterHour);

        replay(m_autoAttendantManager);
    }

    public void testRepresentXml() throws Exception {
        AutoAttendantsResource resource = new AutoAttendantsResource();
        resource.setAutoAttendantManager(m_autoAttendantManager);

        Representation representation = resource.represent(new Variant(MediaType.TEXT_XML));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("autoattendants.rest.test.xml"));
        assertEquals(expected, generated);
    }

    public void testRepresentJson() throws Exception {
        AutoAttendantsResource resource = new AutoAttendantsResource();
        resource.setAutoAttendantManager(m_autoAttendantManager);

        Representation representation = resource.represent(new Variant(MediaType.APPLICATION_JSON));

        StringWriter writer = new StringWriter();
        representation.write(writer);

        String generated = writer.toString();
        String expected = IOUtils.toString(getClass().getResourceAsStream("autoattendants.rest.test.json"));
        assertEquals(expected, generated);
    }

}
