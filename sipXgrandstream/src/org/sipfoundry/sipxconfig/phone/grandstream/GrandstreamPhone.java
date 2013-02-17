/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public class GrandstreamPhone extends Phone {
    private static final Log LOG = LogFactory.getLog(GrandstreamPhone.class);
    private static final String APPLICATION_OCTET_STREAM = "application/octet-stream";
    private static final String DAYLIGHT_SETTING = "phone/P75";
    private static final String GXW_TIMEZONE_SETTING = "gateway/P246";
    private static final String USERID_PATH = "port/P35-P404-P504-P604-P1704-P1804";
    private static final String HT_USERID_PATH = "port/P35-P735";
    private static final String GXW_USERID_PATH = "port/P4060-P4061-P4062-P4063-P4064-P4065-P4066-P4067"
        + "-P4068-P4069-P4070-P4071-P4072-P4073-P4074-P4075-P4076-P4077-P4078-P4079-P4080-P4081-P4082-P4083";
    private static final String AUTHID_PATH = "port/P36-P405-P505-P605-P1705-P1805";
    private static final String HT_AUTHID_PATH = "port/P36-P736";
    private static final String GXW_AUTHID_PATH = "port/P4090-P4091-P4092-P4093-P4094-P4095-P4096-P4097"
        + "-P4098-P4099-P4100-P4101-P4102-P4103-P4104-P4105-P4106-P4107-P4108-P4109-P4110-P4111-P4112-P4113";
    private static final String PASSWORD_PATH = "port/P34-P406-P506-P606-P1706-P1806";
    private static final String HT_PASSWORD_PATH = "port/P34-P734";
    private static final String GXW_PASSWORD_PATH = "port/P4120-P4121-P4122-P4123-P4124-P4125-P4126-P4127"
        + "-P4128-P4129-P4130-P4131-P4132-P4133-P4134-P4135-P4136-P4137-P4138-P4139-P4140-P4141-P4142-P4143";
    private static final String ACCOUNT_NAME_PATH = "port/P270-P417-P517-P617-P1717-P1817";
    private static final String DISPLAY_NAME_PATH = "port/P3-P407-P507-P607-P1707-P1807";
    private static final String HT_DISPLAY_NAME_PATH = "port/P3-P703";
    private static final String GXW_DISPLAY_NAME_PATH = "port/P4180-P4181-P4182-P4183-P4184-P4185-P4186-P4187"
        + "-P4188-P4189-P4190-P4191-P4192-P4193-P4194-P4195-P4196-P4197-P4198-P4199-P4252-P4253-P5254-P4255";
    private static final String REGISTRATION_SERVER_PATH = "port/P47-P402-P502-P602-P1702-P1802";
    private static final String LINE_ACTIVE_PATH = "port/P271-P401-P501-P601-P1701-P1801";
    private static final String HT_REGISTRATION_SERVER_PATH = "port/P47-P747-P47-P747";
    private static final String GXW_REGISTRATION_SERVER_PATH = "account-proxy/P47";
    private static final String GXW_HUNTGROUP_PATH = "port/P4300-P4301-P4302-P4303-P4304-P4305-P4306-P4307-P4308"
        + "-P4309-P4310-P4311-P4312-P4313-P4314-P4315-P4316-P4317-P4318-P4319-P4320-P4321-P4322-P4323";
    private static final String VOICEMAIL_PATH = "port/P33-P426-P526-P626-P1726-P1826";
    private static final String MOH_URI_PATH = "port/P2350-P2450-P2550-P2650-P2760-P2850";
    private static final String DIRECTED_CALL_PICKUP_PREFIX = "port/P1347-P481-P581-P681-P1781-P1881";
    private static final String DOT = ".";
    private static final String CONFIG_FILE_PREFIX = "gs_config/";
    private static final String SEPARATOR = "/";
    private static final String THREE = "3";
    private static final String ZERO = "0";
    private static final String MULTIPURPOSEKEYS_P323 = "multipurposekeys/P323";
    private static final String MULTIPURPOSEKEYS_P301 = "multipurposekeys/P301";
    private static final String MULTIPURPOSEKEYS_P302 = "multipurposekeys/P302";
    private static final String MULTIPURPOSEKEYS_P303 = "multipurposekeys/P303";
    private static final String MULTIPURPOSEKEYS_P324 = "multipurposekeys/P324";
    private static final String MULTIPURPOSEKEYS_P304 = "multipurposekeys/P304";
    private static final String MULTIPURPOSEKEYS_P305 = "multipurposekeys/P305";
    private static final String MULTIPURPOSEKEYS_P306 = "multipurposekeys/P306";
    private static final String MULTIPURPOSEKEYS_P325 = "multipurposekeys/P325";
    private static final String MULTIPURPOSEKEYS_P307 = "multipurposekeys/P307";
    private static final String MULTIPURPOSEKEYS_P308 = "multipurposekeys/P308";
    private static final String MULTIPURPOSEKEYS_P309 = "multipurposekeys/P309";
    private static final String MULTIPURPOSEKEYS_P326 = "multipurposekeys/P326";
    private static final String MULTIPURPOSEKEYS_P310 = "multipurposekeys/P310";
    private static final String MULTIPURPOSEKEYS_P311 = "multipurposekeys/P311";
    private static final String MULTIPURPOSEKEYS_P312 = "multipurposekeys/P312";
    private static final String MULTIPURPOSEKEYS_P327 = "multipurposekeys/P327";
    private static final String MULTIPURPOSEKEYS_P313 = "multipurposekeys/P313";
    private static final String MULTIPURPOSEKEYS_P314 = "multipurposekeys/P314";
    private static final String MULTIPURPOSEKEYS_P315 = "multipurposekeys/P315";
    private static final String MULTIPURPOSEKEYS_P328 = "multipurposekeys/P328";
    private static final String MULTIPURPOSEKEYS_P316 = "multipurposekeys/P316";
    private static final String MULTIPURPOSEKEYS_P317 = "multipurposekeys/P317";
    private static final String MULTIPURPOSEKEYS_P318 = "multipurposekeys/P318";
    private static final String MULTIPURPOSEKEYS_P329 = "multipurposekeys/P329";
    private static final String MULTIPURPOSEKEYS_P319 = "multipurposekeys/P319";
    private static final String MULTIPURPOSEKEYS_P320 = "multipurposekeys/P320";
    private static final String MULTIPURPOSEKEYS_P321 = "multipurposekeys/P321";
    private static final String MULTIPURPOSEKEYS_P353 = "multipurposekeys/P353";
    private static final String MULTIPURPOSEKEYS_P354 = "multipurposekeys/P354";
    private static final String MULTIPURPOSEKEYS_P355 = "multipurposekeys/P355";
    private static final String MULTIPURPOSEKEYS_P356 = "multipurposekeys/P356";
    private static final String MULTIPURPOSEKEYS_P357 = "multipurposekeys/P357";
    private static final String MULTIPURPOSEKEYS_P358 = "multipurposekeys/P358";
    private static final String MULTIPURPOSEKEYS_P359 = "multipurposekeys/P359";
    private static final String MULTIPURPOSEKEYS_P360 = "multipurposekeys/P360";
    private static final String MULTIPURPOSEKEYS_P361 = "multipurposekeys/P361";
    private static final String MULTIPURPOSEKEYS_P362 = "multipurposekeys/P362";
    private static final String MULTIPURPOSEKEYS_P363 = "multipurposekeys/P363";
    private static final String MULTIPURPOSEKEYS_P364 = "multipurposekeys/P364";
    private static final String MULTIPURPOSEKEYS_P365 = "multipurposekeys/P365";
    private static final String MULTIPURPOSEKEYS_P366 = "multipurposekeys/P366";
    private static final String MULTIPURPOSEKEYS_P367 = "multipurposekeys/P367";
    private static final String MULTIPURPOSEKEYS_P368 = "multipurposekeys/P368";
    private static final String MULTIPURPOSEKEYS_P369 = "multipurposekeys/P369";
    private static final String MULTIPURPOSEKEYS_P370 = "multipurposekeys/P370";
    private static final String MULTIPURPOSEKEYS_P371 = "multipurposekeys/P371";
    private static final String MULTIPURPOSEKEYS_P372 = "multipurposekeys/P372";
    private static final String MULTIPURPOSEKEYS_P373 = "multipurposekeys/P373";
    private static final String MULTIPURPOSEKEYS_P374 = "multipurposekeys/P374";
    private static final String MULTIPURPOSEKEYS_P375 = "multipurposekeys/P375";
    private static final String MULTIPURPOSEKEYS_P376 = "multipurposekeys/P376";
    private static final String MULTIPURPOSEKEYS_P377 = "multipurposekeys/P377";
    private static final String MULTIPURPOSEKEYS_P378 = "multipurposekeys/P378";
    private static final String MULTIPURPOSEKEYS_P379 = "multipurposekeys/P379";
    private static final String MULTIPURPOSEKEYS_P380 = "multipurposekeys/P380";
    private static final String MULTIPURPOSEKEYS_P381 = "multipurposekeys/P381";
    private static final String MULTIPURPOSEKEYS_P382 = "multipurposekeys/P382";
    private static final String MULTIPURPOSEKEYS_P383 = "multipurposekeys/P383";
    private static final String MULTIPURPOSEKEYS_P384 = "multipurposekeys/P384";
    private static final String MULTIPURPOSEKEYS_P385 = "multipurposekeys/P385";
    private static final String MULTIPURPOSEKEYS_P386 = "multipurposekeys/P386";
    private static final String MULTIPURPOSEKEYS_P387 = "multipurposekeys/P387";
    private static final String MULTIPURPOSEKEYS_P388 = "multipurposekeys/P388";
    private static final String MULTIPURPOSEKEYS_P389 = "multipurposekeys/P389";
    private static final String MULTIPURPOSEKEYS_P390 = "multipurposekeys/P390";
    private static final String MULTIPURPOSEKEYS_P391 = "multipurposekeys/P391";
    private static final String MULTIPURPOSEKEYS_P392 = "multipurposekeys/P392";
    private static final String MULTIPURPOSEKEYS_P393 = "multipurposekeys/P393";
    private static final String MULTIPURPOSEKEYS_P394 = "multipurposekeys/P394";
    private static final String MULTIPURPOSEKEYS_P395 = "multipurposekeys/P395";
    private static final String MULTIPURPOSEKEYS_P396 = "multipurposekeys/P396";

    private static final String[][] GXP_7_BUTTON = {
        {
            MULTIPURPOSEKEYS_P323, MULTIPURPOSEKEYS_P301, MULTIPURPOSEKEYS_P302, MULTIPURPOSEKEYS_P303
        }, {
            MULTIPURPOSEKEYS_P324, MULTIPURPOSEKEYS_P304, MULTIPURPOSEKEYS_P305, MULTIPURPOSEKEYS_P306
        }, {
            MULTIPURPOSEKEYS_P325, MULTIPURPOSEKEYS_P307, MULTIPURPOSEKEYS_P308, MULTIPURPOSEKEYS_P309
        }, {
            MULTIPURPOSEKEYS_P326, MULTIPURPOSEKEYS_P310, MULTIPURPOSEKEYS_P311, MULTIPURPOSEKEYS_P312
        }, {
            MULTIPURPOSEKEYS_P327, MULTIPURPOSEKEYS_P313, MULTIPURPOSEKEYS_P314, MULTIPURPOSEKEYS_P315
        }, {
            MULTIPURPOSEKEYS_P328, MULTIPURPOSEKEYS_P316, MULTIPURPOSEKEYS_P317, MULTIPURPOSEKEYS_P318
        }, {
            MULTIPURPOSEKEYS_P329, MULTIPURPOSEKEYS_P319, MULTIPURPOSEKEYS_P320, MULTIPURPOSEKEYS_P321
        }
    };
    private static final String[][] GXP_18_BUTTON = {
        {
            MULTIPURPOSEKEYS_P323, MULTIPURPOSEKEYS_P301, MULTIPURPOSEKEYS_P302, MULTIPURPOSEKEYS_P303
        }, {
            MULTIPURPOSEKEYS_P324, MULTIPURPOSEKEYS_P304, MULTIPURPOSEKEYS_P305, MULTIPURPOSEKEYS_P306
        }, {
            MULTIPURPOSEKEYS_P325, MULTIPURPOSEKEYS_P307, MULTIPURPOSEKEYS_P308, MULTIPURPOSEKEYS_P309
        }, {
            MULTIPURPOSEKEYS_P326, MULTIPURPOSEKEYS_P310, MULTIPURPOSEKEYS_P311, MULTIPURPOSEKEYS_P312
        }, {
            MULTIPURPOSEKEYS_P327, MULTIPURPOSEKEYS_P313, MULTIPURPOSEKEYS_P314, MULTIPURPOSEKEYS_P315
        }, {
            MULTIPURPOSEKEYS_P328, MULTIPURPOSEKEYS_P316, MULTIPURPOSEKEYS_P317, MULTIPURPOSEKEYS_P318
        }, {
            MULTIPURPOSEKEYS_P329, MULTIPURPOSEKEYS_P319, MULTIPURPOSEKEYS_P320, MULTIPURPOSEKEYS_P321
        }, {
            MULTIPURPOSEKEYS_P353, MULTIPURPOSEKEYS_P354, MULTIPURPOSEKEYS_P355, MULTIPURPOSEKEYS_P356
        }, {
            MULTIPURPOSEKEYS_P357, MULTIPURPOSEKEYS_P358, MULTIPURPOSEKEYS_P359, MULTIPURPOSEKEYS_P360
        }, {
            MULTIPURPOSEKEYS_P361, MULTIPURPOSEKEYS_P362, MULTIPURPOSEKEYS_P363, MULTIPURPOSEKEYS_P364
        }, {
            MULTIPURPOSEKEYS_P365, MULTIPURPOSEKEYS_P366, MULTIPURPOSEKEYS_P367, MULTIPURPOSEKEYS_P368
        }, {
            MULTIPURPOSEKEYS_P369, MULTIPURPOSEKEYS_P370, MULTIPURPOSEKEYS_P371, MULTIPURPOSEKEYS_P372
        }, {
            MULTIPURPOSEKEYS_P373, MULTIPURPOSEKEYS_P374, MULTIPURPOSEKEYS_P375, MULTIPURPOSEKEYS_P376
        }, {
            MULTIPURPOSEKEYS_P377, MULTIPURPOSEKEYS_P378, MULTIPURPOSEKEYS_P379, MULTIPURPOSEKEYS_P380
        }, {
            MULTIPURPOSEKEYS_P381, MULTIPURPOSEKEYS_P382, MULTIPURPOSEKEYS_P383, MULTIPURPOSEKEYS_P384
        }, {
            MULTIPURPOSEKEYS_P385, MULTIPURPOSEKEYS_P386, MULTIPURPOSEKEYS_P387, MULTIPURPOSEKEYS_P388
        }, {
            MULTIPURPOSEKEYS_P389, MULTIPURPOSEKEYS_P390, MULTIPURPOSEKEYS_P391, MULTIPURPOSEKEYS_P392
        }, {
            MULTIPURPOSEKEYS_P393, MULTIPURPOSEKEYS_P394, MULTIPURPOSEKEYS_P395, MULTIPURPOSEKEYS_P396
        }
    };
    private static final String[][] GXP_24_BUTTON = {
        {
            MULTIPURPOSEKEYS_P323, MULTIPURPOSEKEYS_P301, MULTIPURPOSEKEYS_P302, MULTIPURPOSEKEYS_P303
        }, {
            MULTIPURPOSEKEYS_P324, MULTIPURPOSEKEYS_P304, MULTIPURPOSEKEYS_P305, MULTIPURPOSEKEYS_P306
        }, {
            MULTIPURPOSEKEYS_P325, MULTIPURPOSEKEYS_P307, MULTIPURPOSEKEYS_P308, MULTIPURPOSEKEYS_P309
        }, {
            MULTIPURPOSEKEYS_P326, MULTIPURPOSEKEYS_P310, MULTIPURPOSEKEYS_P311, MULTIPURPOSEKEYS_P312
        }, {
            MULTIPURPOSEKEYS_P327, MULTIPURPOSEKEYS_P313, MULTIPURPOSEKEYS_P314, MULTIPURPOSEKEYS_P315
        }, {
            MULTIPURPOSEKEYS_P328, MULTIPURPOSEKEYS_P316, MULTIPURPOSEKEYS_P317, MULTIPURPOSEKEYS_P318
        }, {
            MULTIPURPOSEKEYS_P329, MULTIPURPOSEKEYS_P319, MULTIPURPOSEKEYS_P320, MULTIPURPOSEKEYS_P321
        }, {
            MULTIPURPOSEKEYS_P353, MULTIPURPOSEKEYS_P354, MULTIPURPOSEKEYS_P355, MULTIPURPOSEKEYS_P356
        }, {
            MULTIPURPOSEKEYS_P357, MULTIPURPOSEKEYS_P358, MULTIPURPOSEKEYS_P359, MULTIPURPOSEKEYS_P360
        }, {
            MULTIPURPOSEKEYS_P361, MULTIPURPOSEKEYS_P362, MULTIPURPOSEKEYS_P363, MULTIPURPOSEKEYS_P364
        }, {
            MULTIPURPOSEKEYS_P365, MULTIPURPOSEKEYS_P366, MULTIPURPOSEKEYS_P367, MULTIPURPOSEKEYS_P368
        }, {
            MULTIPURPOSEKEYS_P369, MULTIPURPOSEKEYS_P370, MULTIPURPOSEKEYS_P371, MULTIPURPOSEKEYS_P372
        }, {
            MULTIPURPOSEKEYS_P373, MULTIPURPOSEKEYS_P374, MULTIPURPOSEKEYS_P375, MULTIPURPOSEKEYS_P376
        }, {
            MULTIPURPOSEKEYS_P377, MULTIPURPOSEKEYS_P378, MULTIPURPOSEKEYS_P379, MULTIPURPOSEKEYS_P380
        }, {
            MULTIPURPOSEKEYS_P381, MULTIPURPOSEKEYS_P382, MULTIPURPOSEKEYS_P383, MULTIPURPOSEKEYS_P384
        }, {
            MULTIPURPOSEKEYS_P385, MULTIPURPOSEKEYS_P386, MULTIPURPOSEKEYS_P387, MULTIPURPOSEKEYS_P388
        }, {
            MULTIPURPOSEKEYS_P389, MULTIPURPOSEKEYS_P390, MULTIPURPOSEKEYS_P391, MULTIPURPOSEKEYS_P392
        }, {
            MULTIPURPOSEKEYS_P393, MULTIPURPOSEKEYS_P394, MULTIPURPOSEKEYS_P395, MULTIPURPOSEKEYS_P396
        }, {
            "multipurposekeys/P1440", "multipurposekeys/P1441", "multipurposekeys/P1442", "multipurposekeys/P1443"
        }, {
            "multipurposekeys/P1444", "multipurposekeys/P1445", "multipurposekeys/P1446", "multipurposekeys/P1447"
        }, {
            "multipurposekeys/P1448", "multipurposekeys/P1449", "multipurposekeys/P1450", "multipurposekeys/P1451"
        }, {
            "multipurposekeys/P1452", "multipurposekeys/P1453", "multipurposekeys/P1454", "multipurposekeys/P1455"
        }, {
            "multipurposekeys/P1456", "multipurposekeys/P1457", "multipurposekeys/P1458", "multipurposekeys/P1459"
        }, {
            "multipurposekeys/P1460", "multipurposekeys/P1461", "multipurposekeys/P1462", "multipurposekeys/P1463"
        }
    };

    private boolean m_isTextFormatEnabled;

    private String m_phonebookLocation = "gs_phonebook/{0}";
    private String m_phonebookFilename = "/phonebook.xml";

    public GrandstreamPhone() {
    }

    @Override
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        SettingExpressionEvaluator evaluator = new GrandstreamSettingExpressionEvaluator(getModel().getModelId());
        return evaluator;
    }

    @Override
    public void initialize() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        GrandstreamDefaults defaults = new GrandstreamDefaults(getPhoneContext().getPhoneDefaults(), speedDial);
        addDefaultBeanSettingHandler(defaults);
        GrandstreamPhonebookDefaults phonebookDefaults = new GrandstreamPhonebookDefaults(getPhoneContext()
                .getPhoneDefaults());
        addDefaultBeanSettingHandler(phonebookDefaults);
        if (speedDial != null) {
            transformSpeedDial(speedDial.getButtons());
        }
    }

    public void transformSpeedDial(List<Button> buttons) {
        if (getGsModel().speedDial()) {
            int maxSize = buttons.size();
            switch (getGsModel().speedDialKeys()) {

            // There are some models that have 0 speed dial keys that support expansion consoles
            case 0:
                if (getGsModel().speedDialExpansion()) {
                    if (maxSize >= 112) {
                        maxSize = 112;
                    }
                     // Time to fill the expansion console P values
                    for (int i = 0; i < maxSize; i++) {
                        Button button = buttons.get(i);
                        setButtonSettings(button, i, 0);
                    }
                }
                break;

            // Grandstream phones with 7 speed dial/BLF buttons
            case 7:
                for (int i = 0; i < 7 && i < buttons.size(); i++) {
                    Button button = buttons.get(i);
                    if (button.isBlf()) {
                        setSettingTypedValue(GXP_7_BUTTON[i][0], THREE);
                    } else {
                        setSettingTypedValue(GXP_7_BUTTON[i][0], ZERO);
                    }
                    setSettingTypedValue(GXP_7_BUTTON[i][1], ZERO);
                    setSettingTypedValue(GXP_7_BUTTON[i][2], button.getLabel());
                    setSettingTypedValue(GXP_7_BUTTON[i][3], button.getNumber());
                }
                // Since the speed dial keys on the phone are all taken up, we must start on the
                // expansion console.
                if (getGsModel().speedDialExpansion() && buttons.size() >= 7) {
                    if (maxSize >= (112 + 7)) {
                        maxSize = 112;
                    } else {
                        maxSize = buttons.size();
                    }
                    // Start at the 7th defined button (i = 7) and work our way up until we reach
                    // the end of the list
                    for (int i = 7; i < maxSize; i++) {
                        Button button = buttons.get(i);
                        setButtonSettings(button, i, 7);
                    }
                }
                break;

            case 18:
                for (int i = 0; i < 18 && i < buttons.size(); i++) {
                    Button button = buttons.get(i);
                    if (button.isBlf()) {
                        setSettingTypedValue(GXP_18_BUTTON[i][0], THREE);
                    } else {
                        setSettingTypedValue(GXP_18_BUTTON[i][0], ZERO);
                    }
                    setSettingTypedValue(GXP_18_BUTTON[i][1], ZERO);
                    setSettingTypedValue(GXP_18_BUTTON[i][2], button.getLabel());
                    setSettingTypedValue(GXP_18_BUTTON[i][3], button.getNumber());
                }
                if (getGsModel().speedDialExpansion() && buttons.size() >= 18) {
                    if (maxSize >= (112 + 18)) {
                        maxSize = 112;
                    }
                    // Start at the 7th defined button (i = 18) and work our way up until we reach
                    // the end of the list
                    for (int i = 18; i < maxSize; i++) {
                        Button button = buttons.get(i);
                        setButtonSettings(button, i, 18);
                    }
                }
                break;

            case 24:
                for (int i = 0; i < 24 && i < buttons.size(); i++) {
                    Button button = buttons.get(i);
                    if (button.isBlf()) {
                        setSettingTypedValue(GXP_24_BUTTON[i][0], THREE);
                    } else {
                        setSettingTypedValue(GXP_24_BUTTON[i][0], ZERO);
                    }
                    setSettingTypedValue(GXP_24_BUTTON[i][1], ZERO);
                    setSettingTypedValue(GXP_24_BUTTON[i][2], button.getLabel());
                    setSettingTypedValue(GXP_24_BUTTON[i][3], button.getNumber());
                }
                if (getGsModel().speedDialExpansion() && buttons.size() >= 24) {
                    if (maxSize >= (112 + 24)) {
                        maxSize = 112;
                    }
                    // Start at the 24th defined button (i = 24) and work our way up until we
                    // reach the end of the list
                    for (int i = 24; i < maxSize; i++) {
                        Button button = buttons.get(i);
                        setButtonSettings(button, i, 24);
                    }
                }
                break;

            default:
                return;
            }
        }
    }

    private void setButtonSettings(Button button, int i, int offset) {
        String expboardkeys = "expboardkeys/P";
        if (button.isBlf()) {
            setSettingTypedValue(expboardkeys + Integer.toString(i + 6001 - offset), THREE);
        } else {
            setSettingTypedValue(expboardkeys + Integer.toString(i + 6001 - offset), ZERO);
        }
        setSettingTypedValue(expboardkeys + Integer.toString(i + 6201 - offset), ZERO);
        setSettingTypedValue(expboardkeys + Integer.toString(i + 6401 - offset), button.getLabel());
        setSettingTypedValue(expboardkeys + Integer.toString(i + 6601 - offset), button.getNumber());
    }

    @Override
    public void initializeLine(Line line) {
        GrandstreamLineDefaults defaults = new GrandstreamLineDefaults(this, line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        if (getGsModel().isHandyTone() && !getGsModel().isHandyTone704()) {
            lineInfo.setDisplayName(line.getSettingValue(HT_DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(HT_USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(HT_PASSWORD_PATH));
            lineInfo.setRegistrationServer(line.getSettingValue(HT_REGISTRATION_SERVER_PATH));
        } else if (getGsModel().isFxsGxw() || getGsModel().isHandyTone704()) {
            lineInfo.setDisplayName(line.getSettingValue(GXW_DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(GXW_USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(GXW_PASSWORD_PATH));
            lineInfo.setHuntgroup(line.getSettingValue(GXW_HUNTGROUP_PATH));
        } else {
            lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
            lineInfo.setUserId(line.getSettingValue(USERID_PATH));
            lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
            lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_SERVER_PATH));
            lineInfo.setVoiceMail(line.getSettingValue(VOICEMAIL_PATH));
        }
        return lineInfo;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
        if (getGsModel().isHandyTone()) {
            line.setSettingValue(HT_DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(HT_USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(HT_PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(HT_REGISTRATION_SERVER_PATH, lineInfo.getRegistrationServer());
        } else if (getGsModel().isFxsGxw()) {
            line.setSettingValue(GXW_DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(GXW_USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(GXW_PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(GXW_HUNTGROUP_PATH, lineInfo.getHuntgroup());
        } else {
            line.setSettingValue(DISPLAY_NAME_PATH, lineInfo.getDisplayName());
            line.setSettingValue(USERID_PATH, lineInfo.getUserId());
            line.setSettingValue(PASSWORD_PATH, lineInfo.getPassword());
            line.setSettingValue(REGISTRATION_SERVER_PATH, lineInfo.getRegistrationServer());
            line.setSettingValue(VOICEMAIL_PATH, lineInfo.getVoiceMail());
        }
    }

    public GrandstreamModel getGsModel() {
        return (GrandstreamModel) getModel();
    }

    public int getLineNumber(int lineId) {
        int lineOffset = 0;
        Iterator ilines = getLines().iterator();
        for (; ilines.hasNext(); lineOffset++) {
            if (((Line) ilines.next()).getId() == lineId) {
                return lineOffset + 1;
            }
        }

        return 0;
    }

    /**
     * Generate files in text format. Won't be usable by phone, but you can use grandstreams
     * config tool to convert manually. This is mostly for debugging
     *
     * @param isTextFormatEnabled true to save as text, default is false
     */
    public void setTextFormatEnabled(boolean isTextFormatEnabled) {
        m_isTextFormatEnabled = isTextFormatEnabled;
    }

    @Override
    public String getModelLabel() {
        return getModel().getLabel();
    }

    @Override
    public String getProfileFilename() {
        String phoneFilename = getSerialNumber();
        return "cfg" + phoneFilename.toLowerCase();
    }

    /*
     * Added phonebook capabilities
     */

    public void setPhonebookLocation(String phonebookLocation) {
        m_phonebookLocation = phonebookLocation;
    }

    public String getPhonebookLocation() {
        return MessageFormat.format(m_phonebookLocation, getSerialNumber());
    }

    /*
     * Set phonebook defaults
     */
    public class GrandstreamPhonebookDefaults {
        private final DeviceDefaults m_defaults;

        GrandstreamPhonebookDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        /*
         * TFTP phonebook download
         */
        @SettingEntry(path = "phonebook/P330")
        public String phonebookDownloadMethod() {
            return "2";
        }

        /*
         * Set download location (gs_phonebook/mac_address)
         */
        @SettingEntry(path = "phonebook/P331")
        public String getPhonebookName() {
            return m_defaults.getTftpServer().getAddress() + SEPARATOR + getPhonebookLocation();
        }
    }

    /*
     * Call phonebook generation
     */
    public ProfileContext getPhonebook() {
        Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
        return new GrandstreamPhonebook(entries);
    }

    static class PhonebookProfile extends Profile {
        public PhonebookProfile(String name) {
            super(name, "text/csv");
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            GrandstreamPhone phone = (GrandstreamPhone) device;
            return phone.getPhonebook();
        }
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes;
        String phonebookLocation = getPhonebookLocation() + m_phonebookFilename;
        PhonebookManager phonebookManager = getPhonebookManager();
        /*
         * If phonebooks are enabled, generate phonebook profile
         */
        if (phonebookManager.getPhonebookManagementEnabled()) {
            profileTypes = new Profile[] {
                new Profile(getProfileFilename(), APPLICATION_OCTET_STREAM),
                new Profile(CONFIG_FILE_PREFIX + getProfileFilename(), APPLICATION_OCTET_STREAM),
                new PhonebookProfile(phonebookLocation)
            };
        } else {
            profileTypes = new Profile[] {
                new Profile(getProfileFilename(), APPLICATION_OCTET_STREAM),
                new Profile(CONFIG_FILE_PREFIX + getProfileFilename(), APPLICATION_OCTET_STREAM)
            };
        }
        return profileTypes;
    }

    public void setDefaultIfExists(Setting sroot, String param, String value) {
        if (sroot.getSetting(param) != null) {
            sroot.getSetting(param).setValue(value);
        }
    }

    public static class GrandstreamDefaults {
        private final DeviceDefaults m_defaults;
        private final SpeedDial m_speedDial;

        GrandstreamDefaults(DeviceDefaults defaults, SpeedDial speedDial) {
            m_defaults = defaults;
            m_speedDial = speedDial;
        }

        /*
         * Set BLF URI if BLF entries are defined
         */
        @SettingEntry(path = "BLF/P134")
        public String getAttendantUri() {
            if (m_speedDial != null && m_speedDial.isBlf()) {
                return m_speedDial.getResourceListId(true);
            } else {
                return "";
            }
        }

        @SettingEntry(path = "upgrade/P234")
        public String configFilePrefix() {
            return CONFIG_FILE_PREFIX;
        }

        private String zoneOffset(String zone, int offset) {
            String zoneOffset;
            int offsetHours = offset / 60;
            int offsetMinutes = offset % 60;
            // Display "-" for a postive offset and "+" for a negative or zero offset
            if (offset > 0) {
                zoneOffset = (zone + "-" + Integer.toString(offsetHours));
            } else {
                zoneOffset = (zone + "%2b" + Integer.toString(-offsetHours));
            }
            if ((offsetMinutes) > 0) {
                zoneOffset = zoneOffset + ":" + Integer.toString(offsetMinutes);
            }
            return zoneOffset;
        }

        private String zoneDstDate(int dstMonth, int dstWeek, int dstDay) {
            String month = Integer.toString(dstMonth + 1);
            String week = Integer.toString(dstWeek);
            String day = Integer.toString(dstDay - 1);
            return (",M" + month + DOT + week + DOT + day);
        }

        private String zoneCustomTime(int standardOffset, int daylightOffset) {
            DeviceTimeZone zone = m_defaults.getTimeZone();
            String timezone = zoneOffset("MTZ", standardOffset) + zoneOffset("MDT", daylightOffset)
                    + zoneDstDate(zone.getStartMonth(), zone.getStartWeek(), zone.getStartDayOfWeek())
                    + zoneDstDate(zone.getStopMonth(), zone.getStopWeek(), zone.getStopDayOfWeek());
            return timezone;
        }

        @SettingEntry(path = DAYLIGHT_SETTING)
        public boolean getUseDaylight() {
            return m_defaults.getTimeZone().getUseDaylight();
        }

        @SettingEntry(path = GXW_TIMEZONE_SETTING)
        public String getGatewayTimeOffset() {
            int offsetInDst;
            if (m_defaults.getTimeZone().getUseDaylight()) {
                offsetInDst = m_defaults.getTimeZone().getOffsetWithDst();
            } else {
                offsetInDst = m_defaults.getTimeZone().getOffset();
            }
            String timezone = zoneCustomTime(m_defaults.getTimeZone().getOffset(), offsetInDst);
            return timezone;
        }

        @SettingEntry(path = "network/P30")
        public String getNtpServer() {
            return m_defaults.getNtpServer();
        }

        @SettingEntry(paths = { GXW_REGISTRATION_SERVER_PATH })
        public String getRegistationServer() {
            return m_defaults.getDomainName();
        }
    }

    public static class GrandstreamLineDefaults {
        private final GrandstreamPhone m_phone;
        private final Line m_line;
        private final DeviceDefaults m_defaults;

        GrandstreamLineDefaults(GrandstreamPhone phone, Line line, DeviceDefaults defaults) {
            m_phone = phone;
            m_line = line;
            m_defaults = defaults;
        }

        @SettingEntry(paths = { USERID_PATH, HT_USERID_PATH, GXW_USERID_PATH, ACCOUNT_NAME_PATH })
        public String getUserId() {
            String userId = null;
            User u = m_line.getUser();
            if (u != null) {
                userId = u.getUserName();
            }

            return userId;
        }

        @SettingEntry(path = DIRECTED_CALL_PICKUP_PREFIX)
        public String getDirectedCallPickupString() {
            return m_defaults.getDirectedCallPickupCode();
        }

        @SettingEntry(path = MOH_URI_PATH)
        public String getMusicOnHoldURI() {
            String mohUri;
            User u = m_line.getUser();
            if (u != null) {
                mohUri = u.getMusicOnHoldUri();
            } else {
                mohUri = m_defaults.getMusicOnHoldUri();
            }

            return SipUri.stripSipPrefix(mohUri);
        }

        @SettingEntry(paths = { AUTHID_PATH, HT_AUTHID_PATH, GXW_AUTHID_PATH })
        public String getAuthId() {
            String userId = null;
            User u = m_line.getUser();
            if (u != null) {
                userId = u.getUserName() + SEPARATOR + m_phone.getSerialNumber();
            }

            return userId;
        }

        @SettingEntry(paths = { PASSWORD_PATH, HT_PASSWORD_PATH, GXW_PASSWORD_PATH })
        public String getPassword() {
            String password = null;
            User u = m_line.getUser();
            if (u != null) {
                password = u.getSipPassword();
            }

            return password;
        }

        @SettingEntry(paths = { DISPLAY_NAME_PATH, HT_DISPLAY_NAME_PATH, GXW_DISPLAY_NAME_PATH })
        public String getDisplayName() {
            String displayName = null;
            User u = m_line.getUser();
            if (u != null) {
                displayName = u.getDisplayName();
            }

            return displayName;
        }

        @SettingEntry(paths = { REGISTRATION_SERVER_PATH, HT_REGISTRATION_SERVER_PATH })
        public String getRegistationServer() {
            return m_phone.getPhoneContext().getPhoneDefaults().getDomainName();
        }

        @SettingEntry(path = GXW_HUNTGROUP_PATH)
        public String getHuntgroup() {
            // Use the line number: line1="1", line2="2", etc.
            int lineNumber = m_phone.getLineNumber(m_line.getId());
            return Integer.toString(lineNumber);
        }

        @SettingEntry(path = VOICEMAIL_PATH)
        public String getVoicemail() {
            return m_phone.getPhoneContext().getPhoneDefaults().getVoiceMail();
        }

        @SettingEntry(path = LINE_ACTIVE_PATH)
        public boolean isLineActive() {
            boolean active = !StringUtils.isBlank(getUserId());
            return active;
        }
    }

    public Collection getProfileLines() {
        int lineCount = getModel().getMaxLineCount();
        ArrayList linesSettings = new ArrayList(lineCount);

        Collection lines = getLines();
        int i = 0;
        Iterator ilines = lines.iterator();
        for (; ilines.hasNext() && (i < lineCount); i++) {
            linesSettings.add(((Line) ilines.next()).getSettings());
        }

        // copy in blank lines of all unused lines
        for (; i < lineCount; i++) {
            Line line = createLine();
            linesSettings.add(line.getSettings());
        }

        return linesSettings;
    }

    @Override
    protected ProfileContext createContext() {
        return new GrandstreamProfileContext(this, m_isTextFormatEnabled);
    }

    @Override
    public void restart() {
        sendAuthorizedCheckSyncToFirstLine();
    }

    static class GrandstreamSettingExpressionEvaluator implements SettingExpressionEvaluator {
        private final String m_model;

        public GrandstreamSettingExpressionEvaluator(String model) {
            m_model = model;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_model.matches(expression);
        }
    }
}
