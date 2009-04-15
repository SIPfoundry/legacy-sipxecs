/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import java.io.File;

import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for linksys 941/942,ata2102/3102,spa8000
 */
public abstract class Linksys extends Phone {

    public static final String PORT = "port";

    public static final String SIP = "sip";

    protected Linksys() {
    }

    @Override
    public LinksysModel getModel() {
        return (LinksysModel) super.getModel();
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    /**
     * Copy common configuration file.
     */
    @Override
    protected void copyFiles(ProfileLocation location) {
        LinksysModel model = getModel();
        String defaultConfigName = model.getDefaultConfigName();
        if (null == defaultConfigName) {
            return;
        }
        String sourceDefaultName = model.getModelDir() + File.separator + "default.cfg";
        getProfileGenerator().copy(location, sourceDefaultName, defaultConfigName);
    }

    @Override
    public String getProfileFilename() {
        String getSerialNumber = getSerialNumber();
        return "spa" + getSerialNumber.toUpperCase() + ".cfg";
    }
}
