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

import javax.xml.bind.annotation.XmlRootElement;

import org.sipfoundry.commons.util.HolidayPeriod;
import org.sipfoundry.sipxconfig.dialplan.attendant.Holiday;

@XmlRootElement(name = "holidayPeriods")
public class HolidayBean {
    private List<HolidayPeriodBean> m_holidayPeriods;

    public static HolidayBean convertHolidayBean(Holiday holiday) {
        List<HolidayPeriodBean> holidayPeriodList = new ArrayList<HolidayPeriodBean>();
        for (HolidayPeriod hPeriod : holiday.getPeriods()) {
            holidayPeriodList.add(HolidayPeriodBean.convertHolidayPeriod(hPeriod));
        }
        HolidayBean holidayBean = new HolidayBean();
        holidayBean.setHolidayPeriods(holidayPeriodList);
        return holidayBean;
    }

    public void setHolidayPeriods(List<HolidayPeriodBean> holidayPeriods) {
        m_holidayPeriods = holidayPeriods;
    }

    public List<HolidayPeriodBean> getHolidayPeriods() {
        if (m_holidayPeriods == null) {
            return new ArrayList<HolidayPeriodBean>();
        }
        return m_holidayPeriods;
    }

}
