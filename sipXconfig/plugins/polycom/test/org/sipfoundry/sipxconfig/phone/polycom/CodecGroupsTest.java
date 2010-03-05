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

import static org.apache.commons.lang.StringUtils.join;
import static org.apache.commons.lang.StringUtils.trimToNull;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Set;

import org.dom4j.DocumentException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;

import junit.framework.TestCase;

/**
 * Checks that each Polycom codec group has the correct codec options, and the correct
 * ordered list of default selected codecs.
 */
public class CodecGroupsTest extends TestCase {

    /**
     * Test the option list and default selected values for every single Polycom group.
     *
     * @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
     */
    public void testAllCodecGroups() throws DocumentException {

        for (CodecGroupType codec_group : CodecGroupType.values() ) {
            assertCodecGroup(codec_group);
        }
    }

    private void assertCodecGroup(CodecGroupType codecGroup) throws DocumentException {

        // Initialize a phone with the codec group under test.
        PolycomModel model = new PolycomModel();
        Set<String> features = new HashSet<String>();
        features.add(String.format("%s_CodecPref", codecGroup));
        model.setSupportedFeatures(features);
        PolycomPhone phone = new PolycomPhone();
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone, new LinkedList<User>());

        // The adaptor setting for the multi-enum setting.
        Setting codec_adaptor = phone.getSettings().getSetting("voice/codecPref/" + codecGroup);
        assertNotNull(String.format("Failed to get the '%s' codec group Setting.", codecGroup), codec_adaptor);

        // The actual multi-enum codec options setting type.
        MultiEnumSetting codec_type = (MultiEnumSetting) codec_adaptor.getType();
        Collection<String> actual_options = codec_type.getEnums().values();

        // Subsequent assert messages will be inaccurate if the real problem is duplicate entries.
        assertEquals(String.format("The '%s' codec group contains one or more duplicate entries.", codecGroup),
                (new HashSet<String>(actual_options)).size(), actual_options.size());


        // *** 1. Test the set of available codec options. ***

        // The *expected* codec options.
        HashSet<String> expected_options = new HashSet<String>(CODECGROUP_OPTION_MAP.get(codecGroup));

        // Loop though each *actual* option that the setting is offering.
        HashSet<String> unexpected_actual_options = new HashSet<String>();
        for (String actual_option : actual_options ) {

            // Attempt to remove this *actual* option from the *expected* list, but record it if it
            // was not actually found.
            if (!expected_options.remove(actual_option)) {
                unexpected_actual_options.add(actual_option);
            }
        }

        // Were any of the expected options not actually found?
        String message = String.format("The following '%s' codec group is missing the following options: %s.",
                codecGroup, expected_options);
        assertTrue(message, expected_options.isEmpty());

        // Were any of the actual options unexpected?
        message = String.format("The following '%s' codec group options were not expected: %s.",
                codecGroup, unexpected_actual_options);
        assertTrue(message, unexpected_actual_options.isEmpty());


        // *** 2. Test the set and preference order of default selected codecs. ***
        String assumed_separator = "|";
        String actual_selected = trimToNull(join((Collection<String>)codec_adaptor.getTypedValue(), assumed_separator));
        String expected_selected = trimToNull(join(CODECGROUP_SELECTED_MAP.get(codecGroup), assumed_separator));
        assertEquals(String.format("The '%s' codec group's default selected codecs are incorrect.", codecGroup),
                expected_selected, actual_selected);
    }

    public static enum CodecGroupType {
        OTHERS("OTHERS"),
        IP_300("IP_300"),
        IP_650("IP_650"),
        IP_4000("IP_4000"),
        IP_6000("IP_6000"),
        IP_7000("IP_7000"),
        VVX_1500("VVX_1500");

        @SuppressWarnings("unused")
        private String m_name;

        private CodecGroupType(String name) {
            m_name = name;
        }
    }

    /**
     * The correct full list of codec options for the specified Polycom codec group.
     *
     * Note that this may be a subset of the options actually supported by the Polycom codec group.
     * For example, we have not (yet) bothered to list the iLBC codec (XX-6596.)
     *
     * @see http://track.sipfoundry.org/browse/XX-6596: Polycom 3.2.0: Add iLBC codec support
     * @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
     */
    private static final HashMap<CodecGroupType, Collection<String>> CODECGROUP_OPTION_MAP;
    static {
        CODECGROUP_OPTION_MAP = new HashMap<CodecGroupType, Collection<String>>();

        HashSet<String> OTHERS_SET = new HashSet<String>();
        OTHERS_SET.add("G711Mu");
        OTHERS_SET.add("G711A");
        OTHERS_SET.add("G729AB");
        CODECGROUP_OPTION_MAP.put(CodecGroupType.OTHERS, OTHERS_SET);

        // IP_300 has the same options as OTHERS.  (But NOT if we ever implement XX-6596.)
        CODECGROUP_OPTION_MAP.put(CodecGroupType.IP_300, OTHERS_SET);

        HashSet<String> IP_650_SET = new HashSet<String>(OTHERS_SET);
        IP_650_SET.add("G722");
        CODECGROUP_OPTION_MAP.put(CodecGroupType.IP_650, IP_650_SET);

        // IP_4000 has the same options as OTHERS.  (But NOT if we ever implement XX-6596.)
        CODECGROUP_OPTION_MAP.put(CodecGroupType.IP_4000, OTHERS_SET);

        HashSet<String> IP_6000_SET = new HashSet<String>(IP_650_SET);
        IP_6000_SET.add("G7221C.24kbps");
        IP_6000_SET.add("G7221C.32kbps");
        IP_6000_SET.add("G7221C.48kbps");
        IP_6000_SET.add("Siren14.24kbps");
        IP_6000_SET.add("Siren14.32kbps");
        IP_6000_SET.add("Siren14.48kbps");
        IP_6000_SET.add("G7221.16kbps");
        IP_6000_SET.add("G7221.24kbps");
        IP_6000_SET.add("G7221.32kbps");
        CODECGROUP_OPTION_MAP.put(CodecGroupType.IP_6000, IP_6000_SET);

        HashSet<String> IP_7000_SET = new HashSet<String>(IP_6000_SET);
        IP_7000_SET.add("Siren22.32kbps");
        IP_7000_SET.add("Siren22.48kbps");
        IP_7000_SET.add("Siren22.64kbps");
        IP_7000_SET.add("Lin16.16ksps");
        IP_7000_SET.add("Lin16.32ksps");
        IP_7000_SET.add("Lin16.48ksps");
        CODECGROUP_OPTION_MAP.put(CodecGroupType.IP_7000, IP_7000_SET);

        HashSet<String> VVX_1500_SET = new HashSet<String>(IP_6000_SET);
        VVX_1500_SET.add("Lin16.8ksps");
        VVX_1500_SET.add("Lin16.16ksps");
        VVX_1500_SET.add("Lin16.32ksps");
        VVX_1500_SET.add("Lin16.44ksps");
        VVX_1500_SET.add("Lin16.48ksps");
        CODECGROUP_OPTION_MAP.put(CodecGroupType.VVX_1500, VVX_1500_SET);
    }

    /**
     * The correct full list of default selected codecs for the specified Polycom codec group.
     * The AbstractSequentialList value of the Map is ordered by preference.
     *
     * @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
     */
    private static final HashMap<CodecGroupType, ArrayList<String>> CODECGROUP_SELECTED_MAP;
    static {
        CODECGROUP_SELECTED_MAP = new HashMap<CodecGroupType, ArrayList<String>>();

        ArrayList<String> OTHERS_LIST = new ArrayList<String>();
        OTHERS_LIST.add("G711Mu");
        OTHERS_LIST.add("G711A");
        OTHERS_LIST.add("G729AB");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.OTHERS, OTHERS_LIST);

        // IP_300 has the same selected codecs as OTHERS.  (But maybe not if we ever implement XX-6596.)
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.IP_300, OTHERS_LIST);

        ArrayList<String> IP_650_LIST = new ArrayList<String>();
        IP_650_LIST.add("G722");
        IP_650_LIST.add("G711Mu");
        IP_650_LIST.add("G711A");
        IP_650_LIST.add("G729AB");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.IP_650, IP_650_LIST);

        ArrayList<String> IP_4000_LIST = new ArrayList<String>();
        IP_4000_LIST.add("G711Mu");
        IP_4000_LIST.add("G711A");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.IP_4000, IP_4000_LIST);

        ArrayList<String> IP_6000_LIST = new ArrayList<String>();
        IP_6000_LIST.add("G7221C.48kbps");
        IP_6000_LIST.add("Siren14.48kbps");
        IP_6000_LIST.add("G722");
        IP_6000_LIST.add("G7221.32kbps");
        IP_6000_LIST.add("G711Mu");
        IP_6000_LIST.add("G711A");
        IP_6000_LIST.add("G729AB");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.IP_6000, IP_6000_LIST);

        ArrayList<String> IP_7000_LIST = new ArrayList<String>();
        IP_7000_LIST.add("Siren22.64kbps");
        IP_7000_LIST.add("G7221C.48kbps");
        IP_7000_LIST.add("Siren14.48kbps");
        IP_7000_LIST.add("G722");
        IP_7000_LIST.add("G7221.32kbps");
        IP_7000_LIST.add("G711Mu");
        IP_7000_LIST.add("G711A");
        IP_7000_LIST.add("G729AB");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.IP_7000, IP_7000_LIST);

        ArrayList<String> VVX_1500_LIST = new ArrayList<String>();
        VVX_1500_LIST.add("G7221C.48kbps");
        VVX_1500_LIST.add("G7221.32kbps");
        VVX_1500_LIST.add("G722");
        VVX_1500_LIST.add("G711Mu");
        VVX_1500_LIST.add("G711A");
        VVX_1500_LIST.add("G729AB");
        CODECGROUP_SELECTED_MAP.put(CodecGroupType.VVX_1500, VVX_1500_LIST);
    }

    /**
     * For each codec group, ensure this unit test's recorded default selected codecs is
     * indeed a subset of the recorded full options list for that codec group.
     *
     * This is really an  double-check of this test case's data, to ensure false errors are
     * note reported in the other test methods.  It doesn't actually test anything outside
     * of this class.
     *
     * @see http://wiki.sipfoundry.org/display/xecsuser/Polycom#Polycom-Codecgroup
     */
    public void testCodecGroupsSelectedSubsetOfOptions() {

        assertEquals(CODECGROUP_OPTION_MAP.size(), CODECGROUP_SELECTED_MAP.size());

        for (CodecGroupType codec_group : CodecGroupType.values() ) {
            assertTrue( CODECGROUP_OPTION_MAP.get(codec_group).containsAll(CODECGROUP_SELECTED_MAP.get(codec_group)));
        }
    }
}
