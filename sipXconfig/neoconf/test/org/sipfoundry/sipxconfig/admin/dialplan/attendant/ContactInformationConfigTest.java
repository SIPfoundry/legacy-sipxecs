/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;

import org.sipfoundry.sipxconfig.common.DaoUtils;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.acd.BeanWithSettingsTestCase;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;

public class ContactInformationConfigTest extends BeanWithSettingsTestCase {
    private final Conference m_conf = new Conference();

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        initializeBeanWithSettings(m_conf);
    }

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

        m_conf.setOwner(u1);
        m_conf.setName("conf1");
        m_conf.setExtension("111");
        m_conf.getSettings().getSetting(Conference.PARTICIPANT_CODE).setValue("1234");

        User u2 = new User();
        u2.setUserName("201");
        AddressBookEntry abe2 = new AddressBookEntry();
        u2.setAddressBookEntry(abe2);

        User u3 = new User();
        u3.setUserName("202");
        AddressBookEntry abe3 = new AddressBookEntry();
        abe3.setUseBranchAddress(true);
        Branch branch = new Branch();
        Address addr = new Address();
        addr.setStreet("1, Branch Street");
        branch.setAddress(addr);
        u3.setBranch(branch);
        u3.setAddressBookEntry(abe3);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        expectLastCall().andReturn(Arrays.asList(u1, u2, u3));

        ConferenceBridgeContext bridgeContext = createMock(ConferenceBridgeContext.class);
        bridgeContext.findConferencesByOwner(u1);
        expectLastCall().andReturn(Arrays.asList(m_conf)).once().andReturn(new ArrayList<Conference>()).times(2);
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
