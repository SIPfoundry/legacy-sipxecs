/*
*
*
* Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
* Contributors retain copyright to elements licensed under a Contributor Agreement.
* Licensed to the User under the LGPL license.
*
* $
*/
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Collection;
import java.util.HashSet;
import java.util.LinkedList;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;
import org.dom4j.DocumentException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.polycom.CodecGroupsTest.CodecGroupType;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

/**
* Checks that each Polycom model is using only the expected codec group.
*/
public class CodecConfigurationForModelTest extends TestCase {

    private PolycomPhone m_phone;



    /**
* Test the codec preferences for every single (sipXconfig) Polycom model.
* @throws DocumentException
*
* @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
*/
    public void testCodecConfigurationForAllModels() throws Exception {

        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom321");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom330");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom331");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom430");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom450");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom550");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom600");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom650");
        assertCodecConfigurationForModel(CodecGroupType.IP_4000, "polycom4000");
        assertCodecConfigurationForModel(CodecGroupType.IP_5000, "polycom5000");
        assertCodecConfigurationForModel(CodecGroupType.IP_6000, "polycom6000");
        assertCodecConfigurationForModel(CodecGroupType.IP_7000, "polycom7000");
        assertCodecConfigurationForModel(CodecGroupType.VVX_1500, "polycomVVX1500");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom335");
    }

    private void assertCodecConfigurationForModel(CodecGroupType codecGroup, String phoneModelId) throws Exception {

        // Initialize the phone.
        m_phone = new PolycomPhone();
        m_phone.setModel(PolycomXmlTestCase.phoneModelBuilder(phoneModelId, getClass()));
        PhoneTestDriver.supplyTestData(m_phone, new LinkedList<User>());

        // Each model belongs to exactly one codec group.
        Collection<Setting> codecPref = m_phone.getSettings().getSetting("voice/codecPref").getValues();
        assertTrue(String.format("The '%s' model does not have a codec group.", phoneModelId), 0 != codecPref.size());
        String str_groups = "";
        for (Setting s : codecPref) {
            str_groups += "-" + s.getName();
        }
        assertEquals(String.format("The '%s' model has more than one codec group ('%s'):", phoneModelId, str_groups),
              1, codecPref.size());

        // Make sure it's the correct codec group.
        Setting codec_adaptor = codecPref.iterator().next();
        assertEquals(String.format("The '%s' model has the wrong codec group:", phoneModelId),
                codecGroup.toString(), codec_adaptor.getName());

        // Collect the major types of the supported codec options. (Remove the minor bit/sample rates.)
        HashSet<String> major_supported_codecs = new HashSet<String>();
        Collection<String> options = ((MultiEnumSetting) codec_adaptor.getType()).getEnums().values();
        for (String option : options) {
            int i = option.indexOf('.');
            if (-1 != i) {
                option = option.substring(0, i);
            }
            major_supported_codecs.add(StringUtils.remove(option,"_"));
        }

        // Loop though the audioProfiles for the model. There should be one for major supported codec type.
        Collection<Setting> audioProfile = m_phone.getSettings().getSetting("voice/audioProfile").getValues();
        for (Setting s : audioProfile) {
            assertTrue(String.format("The '%s' model has an audioProfile for unsupported codec type '%s'.",
                    phoneModelId, s.getName()),
                    major_supported_codecs.remove(s.getName()));
        }
        assertEquals(String.format("The '%s' model is missing an audioProfile for the following supported code type(s): %s.",
                phoneModelId, major_supported_codecs),
                0, major_supported_codecs.size());
    }

}