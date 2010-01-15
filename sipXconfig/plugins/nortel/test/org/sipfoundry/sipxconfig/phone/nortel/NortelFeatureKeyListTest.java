/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;

import java.util.ArrayList;

import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;

import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Tests the Contents of the FeatureKeyList profile
 * Line count for each user (speeddials.size() will give the number of speedials of a user)
 * get the line count and divide by 6 , and do assertequals to size of speedials
 */
public class NortelFeatureKeyListTest extends TestCase{
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    @Before
    public void setUp() throws Exception {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    /**
     * sets the values for SpeedDial buttons
     * verfies the contents of the SpeedDial buttons generated inside
     * featureKeyList profile
     * @throws Exception
     *
     *
     * for correct format have a look at the *.fk profile generated at
     * /home/sipxchange/sipx/BUILD/sipXconfig/web/test-results/profile/tftproot
     * If the file doesnt exist go to the UI and generate the profile first
     *[key]
     *index 1
     *label juser
     *type spdial
     *target juser@sipfoundry.org
     *retrieve YES
     *[key]
     *index 2
     *label juser2
     *type spdial
     *target juser2@sipfoundry.org
     *retrieve YES
     *[key]
     *index 3
     *label juser3
     *type spdial
     *target juser3@sipfoundry.org
     *retrieve YES
     *[key]
     *index 4
     *label home
     *type spdial
     *target 6132222222
     *retrieve YES
     */

    @Test
    public void testNortelFeatureKeyList() throws Exception {


        String userName="juser";
        PhoneModel nortelModel = new PhoneModel("nortel");
        NortelPhone phone = new NortelPhone();
        Integer maxButtons=nortelModel.getMaxLineCount();
        phone.setModel(nortelModel);
        // sets test data for the phone and creates profiles
        PhoneTestDriver.supplyTestData(phone, true);

        List<Button> speedial=new ArrayList<Button>();
        for(Integer i=0;i<maxButtons;i++)
        {
            Button button=new Button();
            button.setLabel(userName+i);
            button.setNumber(userName+i+"@sipfoundry.org");
            speedial.add(button);
        }

        SpeedDial sd=new SpeedDial();
        sd.setButtons(speedial);
        NortelFeatureKeyList featureKeyList=new NortelFeatureKeyList(sd);
        m_pg.generate(m_location,featureKeyList, null, "featurekeyList.fk");

        List<String> list = IOUtils.readLines(m_location.getReader());


        // 6 lines per button so numOfButtons * 6

        Integer numOfButtons=speedial.size();
        assertEquals(numOfButtons * 6, list.size());

        Integer k=0;
        for(Integer j=0; j<numOfButtons*6;j=j+6,k++)
        {
            assertEquals("[key]", list.get(j));
            assertEquals("index "+(k+2), list.get(j+1));
            assertEquals("label "+(userName+k),list.get(j+2));
            assertEquals("type spdial", list.get(j+3));
            assertEquals("target "+(userName+k)+"@sipfoundry.org", list.get(j+4));
            assertEquals("retrieve YES", list.get(j+5));
        }

    }

}
