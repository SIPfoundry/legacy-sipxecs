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

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class FirmwareUpdateListener implements DaoEventListener, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(FirmwareUpdateListener.class);
    private static final String GROUP_FW_VERSION = "group.version/firmware.version";
    private static final String SQL_SELECT_GROUP = "select p.phone_id,p.model_id,p.device_version_id,p.serial_number "
            + "from phone p join phone_group pg on pg.phone_id = p.phone_id "
            + "where p.bean_id = 'polycom' and pg.group_id=%d and p.device_version_id != '%s'";
    private static final String SQL_UPDATE = "update phone set device_version_id='%s' where phone_id=%d";
    private JdbcTemplate m_jdbcTemplate;
    private BeanFactory m_beanFactory;

    @Override
    public void onDelete(Object entity) {
    }

    @Override
    public void onSave(Object entity) {
        if (entity instanceof Group) {
            Group g = (Group) entity;
            if (Phone.GROUP_RESOURCE_ID.equals(g.getResource())) {
                String fwversion = g.getSettingValue(GROUP_FW_VERSION);
                if (StringUtils.isNotEmpty(fwversion)) {
                    String versionId = String.format("polycom%s", fwversion);
                    final List<Integer> ids = new LinkedList<Integer>();
                    final List<String> models = new LinkedList<String>();
                    final List<String> serials = new LinkedList<String>();
                    m_jdbcTemplate.query(String.format(SQL_SELECT_GROUP, g.getId(), versionId),
                            new RowCallbackHandler() {

                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    ids.add(rs.getInt("phone_id"));
                                    models.add(rs.getString("model_id"));
                                    serials.add(rs.getString("serial_number"));
                                }
                            });
                    List<String> updates = new ArrayList<String>();
                    for (int i = 0; i < ids.size(); i++) {
                        PolycomModel model = m_beanFactory.getBean(models.get(i), PolycomModel.class);
                        int id = ids.get(i);
                        String serial = serials.get(i);
                        if (ArrayUtils.contains(model.getVersions(), PolycomModel.getPhoneDeviceVersion(versionId))) {
                            LOG.info("Updating " + serial + " to " + versionId);
                            updates.add(String.format(SQL_UPDATE, versionId, id));
                        } else {
                            LOG.debug("Skipping " + serial + " as it doesn't support " + versionId);
                        }
                    }
                    if (updates.size() > 0) {
                        m_jdbcTemplate.batchUpdate(updates.toArray(new String[]{}));
                    }
                }
            }
        }

    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }

    @Override
    public void setBeanFactory(BeanFactory arg0) {
        m_beanFactory = arg0;
    }
}
