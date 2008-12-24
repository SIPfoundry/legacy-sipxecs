/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Element;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class ResourceListsTest extends XMLTestCase {

    public static String[][] BUTTONS_1 = {
        {
            "alpha", "100", "false"
        }, {
            "beta", "102", "true"
        }, {
            "gamma", "104@sipfoundry.org", "true"
        }
    };

    public static String[][] BUTTONS_2 = {
        {
            "alpha", "100", "false"
        }, {
            "beta", "102", "false"
        }, {
            "gamma", "104", "false"
        }
    };

    public static String[][] BUTTONS_3 = {
        {
            "delta", "100", "true"
        }
    };
    
    private List<User> m_users;

    private SpeedDial m_sd1;
    private SpeedDial m_sd2;
    private SpeedDial m_sd3;

    protected void setUp() throws Exception {
        m_users = new ArrayList<User>();
        for (int i = 0; i < 4; i++) {
            User user = new DummyUser(i);
            m_users.add(user);
        }

        m_sd1 = createSpeedDial(BUTTONS_1);
        m_sd1.setUser(m_users.get(1));

        m_sd2 = createSpeedDial(BUTTONS_2);
        m_sd2.setUser(m_users.get(2));
        
        m_sd3 = createSpeedDial(BUTTONS_3);
        m_sd3.setUser(m_users.get(3));
    }

    private SpeedDial createSpeedDial(String[][] buttonsData) {
        List<Button> buttons1 = new ArrayList<Button>();
        for (int i = 0; i < buttonsData.length; i++) {
            String[] buttonData = buttonsData[i];
            Button button = new Button();
            button.setLabel(buttonData[0]);
            button.setNumber(buttonData[1]);
            button.setBlf(Boolean.parseBoolean(buttonData[2]));
            buttons1.add(button);
        }
        SpeedDial sd = new SpeedDial();
        sd.setButtons(buttons1);
        return sd;
    }

    public void testGenerate() throws Exception {
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.loadUsers();
        coreContextControl.andReturn(m_users);
        coreContext.getDomainName();
        coreContextControl.andReturn("example.org").anyTimes();        
        coreContextControl.replay();

        IMocksControl sdmControl = EasyMock.createControl();
        SpeedDialManager sdm = sdmControl.createMock(SpeedDialManager.class);
        sdm.getSpeedDialForUserId(m_users.get(0).getId(), false);
        sdmControl.andReturn(null);
        sdm.getSpeedDialForUserId(m_users.get(1).getId(), false);
        sdmControl.andReturn(m_sd1);
        sdm.getSpeedDialForUserId(m_users.get(2).getId(), false);
        sdmControl.andReturn(m_sd2);
        sdm.getSpeedDialForUserId(m_users.get(3).getId(), false);
        sdmControl.andReturn(m_sd3);
        sdmControl.replay();

        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);

        rl.generate(sdm);
        String generatedXml = rl.getFileContent();

        InputStream referenceXml = getClass().getResourceAsStream("resource-lists.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        coreContextControl.verify();
        sdmControl.verify();
    }

    public void testGenerateEmpty() throws Exception {
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.loadUsers();
        coreContextControl.andReturn(Collections.emptyList());
        coreContextControl.replay();

        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);

        rl.generate(null);
        String fileContent = rl.getFileContent();
        assertXMLEqual(
                "<lists xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01\"/>",
                fileContent);
        coreContextControl.verify();
    }
    
    public void testEmptyLabel() {
        IMocksControl elementControl = EasyMock.createControl();
        Element list = elementControl.createMock(Element.class);
        Element item = elementControl.createMock(Element.class);
        Element name = elementControl.createMock(Element.class);
        list.addElement("resource");        
        elementControl.andReturn(item);
        item.addAttribute("uri", "sip:123@example.org;sipx-noroute=VoiceMail;sipx-userforward=false");
        elementControl.andReturn(item);
        item.addElement("name");
        elementControl.andReturn(name);
        name.setText("123");
        elementControl.replay();

        Button button = new Button(null, "123");
        ResourceLists rl = new ResourceLists();
        rl.createResourceForUser(list, button, "example.org");
        
        elementControl.verify();
    }

    private class DummyUser extends User {
        int m_id;

        public DummyUser(int id) {
            char c = (char) ('a' - 1 + id);
            setUserName("user_" + c);
            m_id = id;
        }

        public Integer getId() {
            return m_id;
        }
    }

}
