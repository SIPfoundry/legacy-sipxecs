/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.common.DaoUtils;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Element;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;

import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

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

    @Override
    protected void setUp() throws Exception {
        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        m_users = new ArrayList<User>();
        for (int i = 0; i < 4; i++) {
            User user = new DummyUser(i);
            user.setPermissionManager(pm);
            m_users.add(user);
        }

        for (int i = 0; i < 2; i++) {
            User user = new DummyImUser(i);
            user.setPermissionManager(pm);
            user.setSettingTypedValue("im/im-account", true);
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
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        coreContextControl.andReturn(m_users);
        coreContext.getDomainName();
        coreContextControl.andReturn("example.org").anyTimes();
        coreContext.loadUserByAlias("102");
        coreContextControl.andReturn(null);
        coreContext.loadUserByAlias("100");
        coreContextControl.andReturn(null);
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
        sdm.getSpeedDialForUserId(m_users.get(4).getId(), false);
        sdmControl.andReturn(null);
        sdm.getSpeedDialForUserId(m_users.get(5).getId(), false);
        sdmControl.andReturn(null);
        sdmControl.replay();

        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);
        rl.setSpeedDialManager(sdm);

        String generatedXml = getFileContent(rl, null);
        InputStream referenceXml = getClass().getResourceAsStream("resource-lists.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        coreContextControl.verify();
        sdmControl.verify();
    }

    public void testGenerateEmpty() throws Exception {
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.loadUsersByPage(0, DaoUtils.PAGE_SIZE);
        coreContextControl.andReturn(Collections.emptyList());
        coreContext.getDomainName();
        coreContextControl.andReturn("example.org");
        coreContextControl.replay();

        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);

        String fileContent = getFileContent(rl, null);
        assertXMLEqual("<lists xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01\"/>",
                fileContent);
        coreContextControl.verify();
    }

    public void testCreateResourceForUser() {
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

        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextControl.andReturn("example.org");
        coreContext.loadUserByAlias("123");
        coreContextControl.andReturn(null);
        coreContextControl.replay();

        Button button = new Button(null, "123");
        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);
        rl.createResourceForUser(list, button);

        elementControl.verify();
    }

    public void testGetUri() {
        User user = new User();
        user.setUserName("abcd");

        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.loadUserByAlias("1234");
        coreContextControl.andReturn(user);
        coreContext.loadUserByAlias("xyz");
        coreContextControl.andReturn(null);
        coreContextControl.replay();

        ResourceLists rl = new ResourceLists();
        rl.setCoreContext(coreContext);

        Button button = new Button();
        button.setNumber("abc@sipfoundry.org");
        assertEquals("sip:abc@sipfoundry.org", rl.buildUri(button, "example.org"));
        button.setNumber("sip:abc@sipfoundry.org");
        assertEquals("sip:abc@sipfoundry.org", rl.buildUri(button, "example.org"));
        button.setNumber("1234");
        assertEquals("sip:abcd@example.org", rl.buildUri(button, "example.org"));
        button.setNumber("xyz");
        assertEquals("sip:xyz@example.org", rl.buildUri(button, "example.org"));

        coreContextControl.verify();
    }

    private class DummyUser extends User {
        int m_id;

        public DummyUser(int id) {
            setModelFilesContext(TestHelper.getModelFilesContext());
            char c = (char) ('a' - 1 + id);
            setUserName("user_" + c);
            m_id = id;
        }

        @Override
        public Integer getId() {
            return m_id;
        }
    }

    private class DummyImUser extends User {
        int m_id;

        public DummyImUser(int id) {
            setUserName("user_name_" + id);
            setImId("user_im_" + id);
            m_id = id;
        }

        @Override
        public Integer getId() {
            return m_id;
        }
    }
}
