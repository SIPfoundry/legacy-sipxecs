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

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.sipxconfig.cdr.Cdr;

@XmlRootElement(name = "Cdrs")
public class CdrList {

    private List<CdrBean> m_cdrs;

    public void setCdrs(List<CdrBean> cdrs) {
        m_cdrs = cdrs;
    }

    @XmlElement(name = "Cdr")
    public List<CdrBean> getCdrs() {
        if (m_cdrs == null) {
            m_cdrs = new ArrayList<CdrBean>();
        }
        return m_cdrs;
    }

    public static CdrList convertCdrList(List<Cdr> cdrs, Locale locale) {
        CdrList list = new CdrList();
        List<CdrBean> cdrList = new LinkedList<CdrBean>();
        DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT, locale);
        for (Cdr cdr : cdrs) {
            cdrList.add(CdrBean.convertCdr(cdr, dateFormat));
        }
        list.setCdrs(cdrList);
        return list;
    }
}
