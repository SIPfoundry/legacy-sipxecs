/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConferenceNameDecoratorTest extends TestCase {

    public void testGetProfileName() {

        IMocksControl settingCtrl = EasyMock.createControl();
        Setting setting = settingCtrl.createMock(Setting.class);
        setting.getProfileName();
        settingCtrl.andReturn("BOSTON_BRIDGE_CONFERENCE_STATUS");
        settingCtrl.andReturn("BOSTON_BRIDGE_CONFERENCE.AOR");
        settingCtrl.andReturn("BOSTON_BRIDGE_CONFERENCE.REMOTE_ADMIT.SECRET");
        settingCtrl.replay();

        Conference conference = new Conference();
        conference.setName("bongo");
        Conference.ConferenceProfileName handler = new Conference.ConferenceProfileName(
                conference);

        assertEquals("BOSTON_BRIDGE_CONFERENCE_STATUS.bongo", handler.getProfileName(setting)
                .getValue());
        assertEquals("BOSTON_BRIDGE_CONFERENCE.bongo.AOR", handler.getProfileName(setting)
                .getValue());
        assertEquals("BOSTON_BRIDGE_CONFERENCE.bongo.REMOTE_ADMIT.SECRET", handler
                .getProfileName(setting).getValue());

        settingCtrl.verify();
    }

}
