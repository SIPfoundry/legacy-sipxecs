/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

public class ContactInformationConfigTest extends TestCase {

    public void testGenerate() throws Exception {
        User u1 = new User();
        u1.setUserName("200");
        u1.setFirstName("Ilya");
        u1.setLastName("Kovalchuk");
        AddressBookEntry abe1 = new AddressBookEntry();
        abe1.setAlternateImId("ilya");
        abe1.setAssistantName("Vyacheslav Kozlov");
        abe1.setAssistantPhoneNumber("0040721234567");
        abe1.setCellPhoneNumber("00407219874563");
        abe1.setCompanyName("Atlanta Thrashers");
        abe1.setFaxNumber("004021321654987");
        abe1.setHomePhoneNumber("0040216534582");
        abe1.setImDisplayName("Kovalchuk17");
        abe1.setImId("ik");
        abe1.setJobDept("Forwards");
        abe1.setJobTitle("Captain");
        abe1.setLocation("Field");
        Address homeAddress1 = new Address();
        homeAddress1.setCity("Atlanta");
        homeAddress1.setStreet("Merrivale Road");
        homeAddress1.setCountry("USA");
        homeAddress1.setState("GA");
        homeAddress1.setZip("90210");
        Address officeAddress1 = new Address();
        officeAddress1.setStreet("Kent Street");
        officeAddress1.setCity("Atlanta");
        officeAddress1.setState("GA");
        officeAddress1.setCountry("USA");
        officeAddress1.setZip("90211");
        officeAddress1.setOfficeDesignation("17");
        abe1.setHomeAddress(homeAddress1);
        abe1.setOfficeAddress(officeAddress1);

        u1.setAddressBookEntry(abe1);

        Conference conf = new Conference();
        conf.setOwner(u1);
        conf.setName("conf1");
        conf.setExtension("111");

        User u2 = new User();
        u2.setUserName("201");
        AddressBookEntry abe2 = new AddressBookEntry();
        u2.setAddressBookEntry(abe2);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(null, null, null, 0, 250, "id", true);
        expectLastCall().andReturn(Arrays.asList(u1, u2));

        ConferenceBridgeContext bridgeContext = createMock(ConferenceBridgeContext.class);
        bridgeContext.findConferencesByOwner(u1);
        expectLastCall().andReturn(Arrays.asList(conf)).once().andReturn(new ArrayList<Conference>()).once();
        replay(bridgeContext);
        replay(coreContext);

        ContactInformationConfig cicfg = new ContactInformationConfig();
        cicfg.setCoreContext(coreContext);
        cicfg.setConferenceBridgeContext(bridgeContext);

        String generatedXml = getFileContent(cicfg, null);
        InputStream referenceXml = getClass().getResourceAsStream("contact-information.test.xml");
        assertEquals(IOUtils.toString(referenceXml), generatedXml);
    }
}
