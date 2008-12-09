/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;

import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.DelegatingSettingModel.InsertValueFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlEscapeValueFilter;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends Phone {
    public static final String BEAN_ID = "polycom";
    public static final String CALL = "call";
    public static final String EMERGENCY = "dialplan/digitmap/routing.1/emergency.1.value";
    static final String REGISTRATION_PATH = "reg/server/1/address";
    static final String REGISTRATION_PORT_PATH = "reg/server/1/port";
    static final String CONTACT_MODE = "contact";
    static final String DISPLAY_NAME_PATH = "reg/displayName";
    static final String PASSWORD_PATH = "reg/auth.password";
    static final String USER_ID_PATH = "reg/address";
    static final String AUTHORIZATION_ID_PATH = "reg/auth.userId";
    static final String TEMPLATE_DIR = "polycom/mac-address.d";

    public PolycomPhone() {
        setDeviceVersion(PolycomModel.VER_2_0);
    }

    public String getDefaultVersionId() {
        DeviceVersion version = getDeviceVersion();
        return version != null ? version.getVersionId() : null;
    }

    /**
     * Default firmware version for polycom phones. Default is 1.6 right now
     *
     * @param defaultVersionId 1.6 or 2.0
     */
    public void setDefaultVersionId(String defaultVersionId) {
        setDeviceVersion(DeviceVersion.getDeviceVersion(PolycomPhone.BEAN_ID + defaultVersionId));
    }

    @Override
    public void initialize() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        PolycomPhoneDefaults phoneDefaults = new PolycomPhoneDefaults(getPhoneContext().getPhoneDefaults(), speedDial);
        addDefaultBeanSettingHandler(phoneDefaults);

        PolycomIntercomDefaults intercomDefaults = new PolycomIntercomDefaults(this);
        addDefaultBeanSettingHandler(intercomDefaults);
    }

    @Override
    public void initializeLine(Line line) {
        PolycomLineDefaults lineDefaults = new PolycomLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(lineDefaults);
    }

    protected ProfileFilter getProfileFilter() {
        return new FormatFilter();
    }

    @Override
    public void generateFiles(ProfileLocation location) {
        ProfileFilter format = getProfileFilter();

        ApplicationConfiguration app = new ApplicationConfiguration(this);
        getProfileGenerator().generate(location, app, format, app.getAppFilename());

        SipConfiguration sip = new SipConfiguration(this);
        getProfileGenerator().generate(location, sip, format, app.getSipFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this);
        getProfileGenerator().generate(location, phone, format, app.getPhoneFilename());

        PhonebookManager phonebookManager = getPhonebookManager();
        if (phonebookManager.getPhonebookManagementEnabled()) {
            Collection<PhonebookEntry> entries = getPhoneContext().getPhonebookEntries(this);
            SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
            DirectoryConfiguration dir = new DirectoryConfiguration(entries, speedDial);
            getProfileGenerator().generate(location, dir, format, app.getDirectoryFilename());
        }
    }

    @Override
    protected void beforeProfileGeneration() {
        InsertValueFilter insert = new InsertValueFilter(XmlEscapeValueFilter.FILTER);
        Setting settings = getSettings();
        settings.acceptVisitor(insert);
        for (Line line : getLines()) {
            Setting lineSettings = line.getSettings();
            lineSettings.acceptVisitor(insert);
        }
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        ApplicationConfiguration app = new ApplicationConfiguration(this);
        location.removeProfile(app.getAppFilename());
        location.removeProfile(app.getSipFilename());
        location.removeProfile(app.getPhoneFilename());
        location.removeProfile(app.getDirectoryFilename());
    }

    /**
     * Polycom 430 1.6.5 would not read files w/being formatted first. Unclear why.
     */
    static class FormatFilter implements ProfileFilter {

        public void copy(InputStream in, OutputStream out) throws IOException {
            SAXReader xmlReader = new SAXReader();
            Document doc;
            try {
                doc = xmlReader.read(in);
            } catch (DocumentException e1) {
                throw new RuntimeException(e1);
            }
            OutputFormat pretty = OutputFormat.createPrettyPrint();
            XMLWriter xml = new XMLWriter(out, pretty);
            xml.write(doc);
        }
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME_PATH, externalLine.getDisplayName());
        line.setSettingValue(USER_ID_PATH, externalLine.getUserId());
        line.setSettingValue(PASSWORD_PATH, externalLine.getPassword());

        // Both userId and authId are required, see XCF-914
        line.setSettingValue(AUTHORIZATION_ID_PATH, externalLine.getUserId());

        line.setSettingValue(REGISTRATION_PATH, externalLine.getRegistrationServer());
        line.setSettingValue(REGISTRATION_PORT_PATH, externalLine.getRegistrationServerPort());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
        lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
        lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        return lineInfo;
    }

    public void restart() {
        sendCheckSyncToFirstLine();
    }
}
