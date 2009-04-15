/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;

public abstract class SbcDeviceAction implements OptionAdapter, IActionListener {

    private Integer m_id;

    private SbcDevice m_sbcDevice;

    public SbcDeviceAction(SbcDevice sbcDevice) {
        m_sbcDevice = sbcDevice;
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }

    public SbcDevice getSbcDevice() {
        return m_sbcDevice;
    }

    public void setSbcDevice(SbcDevice sbcDevice) {
        m_sbcDevice = sbcDevice;
    }

    public Object getValue(Object option, int index_) {
        return this;
    }

    public String getLabel(Object option, int index_) {
        return m_sbcDevice.getName();
    }

    public String squeezeOption(Object option_, int index_) {
        return getClass().getName() + m_sbcDevice.getId();
    }

    public String getMethodName() {
        return null;
    }
}
