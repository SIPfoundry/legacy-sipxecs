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
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.sipfoundry.commons.mongo.MongoConstants.BUTTONS;
import static org.sipfoundry.commons.mongo.MongoConstants.NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.SPEEDDIAL;
import static org.sipfoundry.commons.mongo.MongoConstants.URI;
import static org.sipfoundry.commons.mongo.MongoConstants.USER;
import static org.sipfoundry.commons.mongo.MongoConstants.USER_CONS;

public class SpeedDials extends AbstractDataSetGenerator {
    private static final String IM_ENABLED = "im_enabled";
    private static final String QUERY = "SELECT u.user_id, u.user_name, v.value as im_enabled, vs.value as subscribe, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv on gs.group_id = sv.value_storage_id "
            + "inner join user_group ug on gs.group_id = ug.group_id where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im/im-account' AND sv.value='1') as group_im_enabled "
            + "from Users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='im/im-account' left join setting_value vs on u.value_storage_id = vs.value_storage_id "
            + "AND vs.path='permission/application/subscribe-to-presence' WHERE u.user_type='C' ORDER BY u.user_id;";
    private SpeedDialManager m_speedDialManager;
    private JdbcTemplate m_jdbcTemplate;

    @Override
    protected DataSet getType() {
        return DataSet.SPEED_DIAL;
    }

    @Override
    public void generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            DBObject speedDialDBO = new BasicDBObject();
            SpeedDial speedDial = m_speedDialManager.getSpeedDialForUser(user, false);
            if (speedDial != null) {
                speedDialDBO.put(USER, speedDial.getResourceListId(false));
                speedDialDBO.put(USER_CONS, speedDial.getResourceListId(true));
                List<DBObject> buttonsList = new ArrayList<DBObject>();
                List<Button> buttons = speedDial.getButtons();
                for (Button button : buttons) {
                    if (!button.isBlf()) {
                        continue;
                    }
                    DBObject buttonDBO = new BasicDBObject();
                    buttonDBO.put(URI, buildUri(button.getNumber(), getSipDomain()));
                    String name = StringUtils.defaultIfEmpty(button.getLabel(), button.getNumber());
                    buttonDBO.put(NAME, name);
                    buttonsList.add(buttonDBO);
                }
                if (!buttonsList.isEmpty()) {
                    speedDialDBO.put(BUTTONS, buttonsList);
                }
                top.put(SPEEDDIAL, speedDialDBO);
            } else {
                top.removeField(SPEEDDIAL);
            }
        } else if (entity instanceof SpecialUser) {
            SpecialUser u = (SpecialUser) entity;
            if (u.getUserName().equals(SpecialUserType.XMPP_SERVER.getUserName())) {
                DBObject speedDialDBO = new BasicDBObject();
                speedDialDBO.put(USER, SpeedDial.getResourceListId(u.getUserName(), false));
                speedDialDBO.put(USER_CONS, SpeedDial.getResourceListId(u.getUserName(), true));
                final List<DBObject> buttonsList = new ArrayList<DBObject>();
                m_jdbcTemplate.query(QUERY, new RowCallbackHandler() {

                    @Override
                    public void processRow(ResultSet rs) throws SQLException {
                        if (StringUtils.isNotBlank(rs.getString(IM_ENABLED))
                                && rs.getString(IM_ENABLED).equals("1") || rs.getInt("group_im_enabled") == 1) {
                            String userName = rs.getString("user_name");
                            DBObject buttonDBO = new BasicDBObject();
                            buttonDBO.put(URI, buildUri(userName, getSipDomain()));
                            buttonDBO.put(NAME, userName);
                            buttonsList.add(buttonDBO);
                        }
                    }
                });
                if (!buttonsList.isEmpty()) {
                    speedDialDBO.put(BUTTONS, buttonsList);
                }
                top.put(SPEEDDIAL, speedDialDBO);
            }
        }
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    private String buildUri(String number, String domainName) {
        StringBuilder uri = new StringBuilder();
        if (SipUri.matches(number)) {
            uri.append(SipUri.normalize(number));
        } else {
            // not a URI - check if we have a user
            User user = getCoreContext().loadUserByAlias(number);
            String uname = number;
            if (user != null) {
                // if number matches any known user make sure to use username and not an alias
                uname = user.getUserName();
            }
            uri.append(SipUri.format(uname, domainName, false));
        }
        return uri.toString();
    }

    public void setJdbcTemplate(JdbcTemplate jdbcTemplate) {
        m_jdbcTemplate = jdbcTemplate;
    }
}
