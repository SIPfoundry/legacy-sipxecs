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
    public static final String BEAN_ID = "mitel";

    public MitelPhone() {
        super(BEAN_ID);
    }

    public MitelPhone(MitelModel model) {
        super(model);
    }

    @Override
    public String getPhoneTemplate() {
        return "mitel/mn.txt.vm";
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
    public void generateProfiles() {
        super.generateProfiles();
        // generate some other files
    }

    @Override
    public String getPhoneFilename() {
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
