/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;

public class OpenAcdCommand extends OpenAcdExtension {
    public static List<FreeswitchAction> getDefaultActions(Location location) {
        List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
        actions.add(createAction(FreeswitchAction.PredefinedAction.erlang_sendmsg.toString(),
                "agent_dialplan_listener  " + "openacd@" + location.getFqdn()
                        + " agent_login ${sip_from_user} pstn ${sip_from_uri}"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.answer.toString(), null));
        actions.add(createAction(FreeswitchAction.PredefinedAction.sleep.toString(), "2000"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.hangup.toString(), "NORMAL_CLEARING"));
        return actions;
    }

}
