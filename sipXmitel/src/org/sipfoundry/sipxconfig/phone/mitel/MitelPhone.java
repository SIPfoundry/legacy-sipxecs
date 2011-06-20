/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.mitel;

import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;

public class MitelPhone extends Phone {
    public MitelPhone() {
    }

    @Override
    public void initializeLine(Line line) {
        MitelLineDefaults defaults = new MitelLineDefaults(line, getPhoneContext()
                .getPhoneDefaults());
        line.addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initialize() {
        MitelPhoneDefaults defaults = new MitelPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public String getProfileFilename() {
        String serialNumber = getSerialNumber();
        return "mn_" + serialNumber + ".txt";
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        return info;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
    }
}
