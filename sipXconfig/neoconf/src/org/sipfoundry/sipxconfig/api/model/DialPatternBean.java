/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.model;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;
import org.sipfoundry.sipxconfig.dialplan.DialPattern;

@XmlRootElement(name = "pattern")
@XmlType(propOrder = {
        "prefix", "digits"
        })
@JsonPropertyOrder({
        "prefix", "digits"
    })
public class DialPatternBean {
    private String m_prefix;
    private int m_digits;

    public static DialPatternBean convertPattern(DialPattern pattern) {
        DialPatternBean dpBean = new DialPatternBean();
        dpBean.setDigits(pattern.getDigits());
        dpBean.setPrefix(pattern.getPrefix());
        return dpBean;
    }

    public String getPrefix() {
        return m_prefix;
    }
    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }
    public int getDigits() {
        return m_digits;
    }
    public void setDigits(int digits) {
        m_digits = digits;
    }
}
