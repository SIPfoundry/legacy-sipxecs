/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;

public class FirmwareUpdateListener implements DaoEventListener {
    private static final Log LOG = LogFactory.getLog(FirmwareUpdateListener.class);
    private static final String GROUP_FW_VERSION = "group.version/firmware.version";
    private PhoneContext m_phoneContext;

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    @Override
    public void onDelete(Object entity) {
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Group) {
            Group g = (Group) entity;
            if (Phone.GROUP_RESOURCE_ID.equals(g.getResource())) {
                if (g.getSettingValue(GROUP_FW_VERSION) != null
                        && StringUtils.isNotEmpty(g.getSettingValue(GROUP_FW_VERSION))) {
                    for (Phone phone : m_phoneContext.getPhonesByGroupId(g.getId())) {
                        if (phone instanceof PolycomPhone) {
                            DeviceVersion version = DeviceVersion.getDeviceVersion(PolycomPhone.BEAN_ID
                                    + g.getSettingValue(GROUP_FW_VERSION));
                            if (ArrayUtils.contains(phone.getModel().getVersions(), version)) {
                                LOG.info("Updating " + phone.getSerialNumber() + " to " + version.getVersionId());
                                phone.setDeviceVersion(version);
                                m_phoneContext.storePhone(phone);
                            } else {
                                LOG.info("Skipping " + phone.getSerialNumber() + " as it doesn't support "
                                        + version.getVersionId() + "; model: " + phone.getModelId());
                            }
                        }
                    }
                }
            }
        }

    }
}
