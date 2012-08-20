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

import java.util.ArrayList;
import java.util.List;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

import static org.sipfoundry.commons.mongo.MongoConstants.BUTTONS;
import static org.sipfoundry.commons.mongo.MongoConstants.NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.SPEEDDIAL;
import static org.sipfoundry.commons.mongo.MongoConstants.URI;
import static org.sipfoundry.commons.mongo.MongoConstants.USER;
import static org.sipfoundry.commons.mongo.MongoConstants.USER_CONS;

public class SpeedDials extends AbstractDataSetGenerator {
    private SpeedDialManager m_speedDialManager;

    protected DataSet getType() {
        return DataSet.SPEED_DIAL;
    }

    public boolean generate(Replicable entity, DBObject top) {
        if (!(entity instanceof User)) {
            return false;
        }
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
                buttonDBO.put(URI, buildUri(button, getSipDomain()));
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
        return true;
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }

    private String buildUri(Button button, String domainName) {
        String number = button.getNumber();
        StringBuilder uri = new StringBuilder();
        if (SipUri.matches(number)) {
            uri.append(SipUri.normalize(number));
        } else {
            // not a URI - check if we have a user
            User user = getCoreContext().loadUserByAlias(number);
            if (user != null) {
                // if number matches any known user make sure to use username and not an alias
                number = user.getUserName();
            }
            uri.append(SipUri.format(number, domainName, false));
        }
        return uri.toString();
    }
}
