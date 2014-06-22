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
package org.sipfoundry.sipxconfig.api.impl;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import org.apache.commons.lang.time.DateUtils;


public final class RequestUtils {
    public static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("yyyyMMddHHmm");

    private RequestUtils() {
    }

    public static Date getDefaultEndTime() {
        Calendar now = Calendar.getInstance();
        now.add(Calendar.DAY_OF_MONTH, 1);
        Calendar end = DateUtils.truncate(now, Calendar.DAY_OF_MONTH);
        return end.getTime();
    }

    /**
     *  start a day before end time
     */
    public static Date getDefaultStartTime() {
        Date endTime = getDefaultEndTime();
        Calendar then = Calendar.getInstance();
        then.setTime(endTime);
        then.add(Calendar.DAY_OF_MONTH, -1);
        return then.getTime();
    }

}
