/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
*/

package org.sipfoundry.sipxconfig.callqueue;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;

public class CallQueueCommand extends CallQueueExtension {

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        // TODO: Put some properties for mongo
        props.putAll(super.getMongoProperties(domain));
        return props;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    /* Set extension handling for CallQueue */
    public void setExtension(String extension, String status) {
        if (getConditions() == null) {
            FreeswitchCondition condition = createCondition();
            condition.addAction(createAction("set",
                    String.format("res=${callcenter_config(agent set status agent-${caller_id_number} '%s')}",
                            status)));
            addCondition(condition);
        }
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                condition.setExpression(extension);
            }
        }
    }
}
