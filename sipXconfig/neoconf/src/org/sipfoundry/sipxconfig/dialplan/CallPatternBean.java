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
package org.sipfoundry.sipxconfig.dialplan;

import javax.xml.bind.annotation.XmlEnum;
import javax.xml.bind.annotation.XmlEnumValue;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

import org.codehaus.jackson.annotate.JsonPropertyOrder;

@XmlRootElement(name = "CallPattern")
@XmlType(propOrder = {
        "prefix", "callDigits"
        })
@JsonPropertyOrder({
        "prefix", "callDigits"
    })
public class CallPatternBean {
    private String m_prefix;
    private CallDigitsEnum m_callDigits;

    @XmlType(name = "callDigits")
    @XmlEnum
    public enum CallDigitsEnum {
        @XmlEnumValue(value = "nodigits")
        nodigits,
        @XmlEnumValue(value = "vdigits")
        vdigits,
        @XmlEnumValue(value = "digits")
        digits
    }

    public static CallPatternBean convertCallPattern(CallPattern callPattern) {
        CallPatternBean cpBean = new CallPatternBean();
        cpBean.setPrefix(callPattern.getPrefix());
        cpBean.setCallDigits(CallDigitsEnum.valueOf(callPattern.getDigits().getName()));
        return cpBean;
    }

    public static CallPattern convertToCallPattern(CallPatternBean callPatternBean) {
        CallPattern cp = new CallPattern();
        cp.setPrefix(callPatternBean.getPrefix());
        cp.setDigits(new CallDigits(callPatternBean.getCallDigits().name()));
        return cp;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    public CallDigitsEnum getCallDigits() {
        return m_callDigits;
    }

    public void setCallDigits(CallDigitsEnum callDigits) {
        m_callDigits = callDigits;
    }
}
