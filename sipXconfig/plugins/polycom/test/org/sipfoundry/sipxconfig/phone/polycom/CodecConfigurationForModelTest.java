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
import java.util.List;
import java.util.Set;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.Node;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.polycom.CodecGroupsTest.CodecGroupType;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

import junit.framework.TestCase;

/**
 * Checks that each Polycom model is using only the expected codec group.
 */
public class CodecConfigurationForModelTest extends TestCase {

    private PolycomPhone m_phone;

    @SuppressWarnings("unchecked")
    private static List<Element> getModelBeanPropertyElements(Node model_bean, String name) {
        String xpath = String.format("property[@name=\"%s\"]/set", name);
        return ((Element) model_bean.selectSingleNode(xpath)).elements();
    }

    private static String getModelBeanPropertyValue(Node model_bean, String name) {
        String xpath = String.format("property[@name=\"%s\"]/@value", name);
        return model_bean.selectSingleNode(xpath).getStringValue();
    }

    /**
     * Test the codec preferences for every single (sipXconfig) Polycom model.
     * @throws DocumentException
     *
     * @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
     */
    public void testCodecConfigurationForAllModels() throws DocumentException {

        assertCodecConfigurationForModel(CodecGroupType.IP_300, "polycom300");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom321");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom330");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom331");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom430");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom450");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom500");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom550");
        assertCodecConfigurationForModel(CodecGroupType.OTHERS, "polycom600");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom650");
        assertCodecConfigurationForModel(CodecGroupType.IP_4000, "polycom4000");
        assertCodecConfigurationForModel(CodecGroupType.IP_6000, "polycom6000");
        assertCodecConfigurationForModel(CodecGroupType.IP_7000, "polycom7000");
        assertCodecConfigurationForModel(CodecGroupType.VVX_1500, "polycomVVX1500");
        assertCodecConfigurationForModel(CodecGroupType.IP_650, "polycom335");
    }

    private void assertCodecConfigurationForModel(CodecGroupType codecGroup, String phoneModelId) throws DocumentException {

        // Initialize the phone.
        m_phone = new PolycomPhone();
        m_phone.setModel(phoneModelBuilder(phoneModelId, getClass()));
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

        // Collect the major types of the supported codec options.  (Remove the minor bit/sample rates.)
        HashSet<String> major_supported_codecs = new HashSet<String>();
        Collection<String> options = ((MultiEnumSetting) codec_adaptor.getType()).getEnums().values();
        for (String option : options) {
            int i = option.indexOf('.');
            if (-1 != i) {
                option = option.substring(0, i);
            }
            major_supported_codecs.add(option);
        }

        // Loop though the audioProfiles for the model.  There should be one for major supported codec type.
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

    /**
     * Builds a PolycomModel bean for the specified Model ID.
     *
     * This method is not specific to Codec Options testing.  It could be used generally for
     * other PolycomPhone testing.  (Though are definitely some bean properties that are
     * not being populated.)
     *
     * @throws DocumentException
     */
    public static PolycomModel phoneModelBuilder(String phoneModelId, Class klass) throws DocumentException {

        PolycomModel model = new PolycomModel();
        model.setModelId(phoneModelId);

        Document beans_document = XmlUnitHelper.loadDocument(klass, "polycom-models.beans.xml");

        // Find the bean whose ID matches the specified phone model.
        Node model_bean = beans_document.selectSingleNode(String.format("/beans/bean[@id=\"%s\"]", phoneModelId));
        assertNotNull(String.format("Failed to find a bean with ID '%s'.", phoneModelId), model_bean);

        // Set the properties.
        model.setLabel(getModelBeanPropertyValue(model_bean, "label"));
        model.setMaxLineCount(Integer.parseInt(getModelBeanPropertyValue(model_bean, "maxLineCount")));
        Set<String> features = new HashSet<String>();
        for (Element supportedFeature : getModelBeanPropertyElements(model_bean, "supportedFeatures")) {
            features.add(supportedFeature.getStringValue());
        }
        model.setSupportedFeatures(features);

        return model;
    }
}
