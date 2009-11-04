/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.Date;
import java.util.List;

public interface AlarmHistoryManager {

    List<AlarmEvent> getAlarmEvents(String host, Date startDate, Date endDate);

    List<AlarmEvent> getAlarmEventsByPage(String host, Date startDate, Date endDate, int first, int pageSize);

    /**
     * Apply paging being given the list to page
     * @param contents - the list to apply paging
     * @return page content
     */
    List<AlarmEvent> getAlarmEventsByPage(List<AlarmEvent> contents, int first, int pageSize);
}
