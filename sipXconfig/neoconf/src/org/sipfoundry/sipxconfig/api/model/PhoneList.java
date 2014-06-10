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

import org.sipfoundry.sipxconfig.phone.Phone;

@XmlRootElement(name = "Phones")
public class PhoneList {

    private List<PhoneBean> m_phones;

    public void setPhones(List<PhoneBean> phones) {
        m_phones = phones;
    }

    @XmlElement(name = "Phone")
    public List<PhoneBean> getPhones() {
        if (m_phones == null) {
            m_phones = new ArrayList<PhoneBean>();
        }
        return m_phones;
    }

    public static PhoneList convertPhoneList(List<Phone> phones) {
        List<PhoneBean> phoneList = new ArrayList<PhoneBean>();
        for (Phone phone : phones) {
            phoneList.add(PhoneBean.convertPhone(phone));
        }
        PhoneList list = new PhoneList();
        list.setPhones(phoneList);
        return list;
    }
}
