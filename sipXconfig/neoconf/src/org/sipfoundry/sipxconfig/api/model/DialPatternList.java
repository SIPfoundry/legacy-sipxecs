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

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.dialplan.DialPattern;

@XmlRootElement(name = "DialPatterns")
public class DialPatternList {
    private List<DialPatternBean> m_patterns;

    public void setPatterns(List<DialPatternBean> patterns) {
        m_patterns = patterns;
    }

    public static DialPatternList convertPatternList(List<DialPattern> patterns) {
        List<DialPatternBean> patternList = new ArrayList<DialPatternBean>();
        for (DialPattern pattern : patterns) {
            patternList.add(DialPatternBean.convertPattern(pattern));
        }
        DialPatternList list = new DialPatternList();
        list.setPatterns(patternList);
        return list;
    }

    public static List<DialPattern> convertToPatternList(DialPatternList list) {
        DialPattern pattern = null;
        List<DialPattern> patternList = new ArrayList<DialPattern>();
        for (DialPatternBean bean : list.getPatterns()) {
            pattern = DialPatternBean.convertToPattern(bean);
            patternList.add(pattern);
        }
        return patternList;
    }

    @XmlElement(name = "DialPatterns")
    public List<DialPatternBean> getPatterns() {
        if (m_patterns == null) {
            m_patterns = new ArrayList<DialPatternBean>();
        }
        return m_patterns;
    }

}
