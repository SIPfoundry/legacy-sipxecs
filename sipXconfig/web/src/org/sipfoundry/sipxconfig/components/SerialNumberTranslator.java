/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.translator.StringTranslator;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

/**
 * Converts an input field to lowercase in tapestry
 */
public class SerialNumberTranslator extends StringTranslator {
    private DeviceDescriptor m_descriptor;

    public SerialNumberTranslator(DeviceDescriptor descriptor) {
        m_descriptor = descriptor;
    }

    protected Object parseText(IFormComponent field, ValidationMessages messages, String text) {
        return m_descriptor.cleanSerialNumber(text);
    }
}
