/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.attendant;

import java.util.Hashtable;

import org.sipfoundry.sipxivr.eslrequest.AbstractEslRequestController;

public class AaEslRequestController extends AbstractEslRequestController {
    private static final String RESOURCE_NAME = "org.sipfoundry.attendant.AutoAttendant";
    private String m_scheduleId;
    private String m_aaId;
    private Configuration m_attendantConfig;
    private Schedule m_schedule;

    @Override
    public void extractParameters(Hashtable<String, String> parameters) {
        m_aaId = parameters.get("attendant_id");
        m_scheduleId = parameters.get("schedule_id");
    }

    @Override
    public void loadConfig() {
        initLocalization("AutoAttendant", RESOURCE_NAME);

        // Load the attendant configuration
        m_attendantConfig.update();

        // Load the schedule configuration
        m_schedule = m_attendantConfig.getSchedule(m_scheduleId);
    }

    public String collectDigits(AttendantConfig config, String termChars) {
        return collectDigits(config.getMaximumDigits(), config.getInitialTimeout(), config.getInterDigitTimeout(),
                config.getExtraDigitTimeout(), termChars);
    }

    public String getAttendantId() {
        return m_aaId;
    }

    public Schedule getSchedule() {
        return m_schedule;
    }

    public String getScheduleId() {
        return m_scheduleId;
    }

    public AttendantConfig getAttendantConfig(String id) {
        return m_attendantConfig.getAttendant(id);
    }

    public String getSpecialAttendantId() {
        return m_attendantConfig.getSpecialAttendantId();
    }

    public void setAutoAttendantConfig(Configuration config) {
        m_attendantConfig = config;
    }

}
