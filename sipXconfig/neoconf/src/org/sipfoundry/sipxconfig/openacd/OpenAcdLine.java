/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;

public class OpenAcdLine extends OpenAcdExtension {
    public static final String Q = "queue=";
    public static final String DESTINATION_NUMBER = "destination_number";
    public static final String ALLOW_VOICEMAIL = "allow_voicemail=";
    public static final String EMPTY_STRING = "";

    public static List<FreeswitchAction> getDefaultActions(Location location) {
        List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
        actions.add(createAction(FreeswitchAction.PredefinedAction.answer.toString(), null));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), "domain_name=$${domain}"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), "brand=1"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), Q));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), "allow_voicemail=true"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.erlang_sendmsg.toString(),
                "freeswitch_media_manager  testme@" + location.getHostname() + " inivr ${uuid}"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.playback.toString(), EMPTY_STRING));
        actions.add(createAction(FreeswitchAction.PredefinedAction.erlang.toString(),
                "freeswitch_media_manager:!  testme@" + location.getHostname()));
        return actions;
    }

    public static FreeswitchAction createVoicemailAction(boolean allow) {
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), ALLOW_VOICEMAIL + allow);
    }

    public static FreeswitchAction createQueueAction(String queue) {
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), Q + queue);
    }

    public static FreeswitchAction createPlaybackAction(String path) {
        return createAction(FreeswitchAction.PredefinedAction.playback.toString(), path);
    }

}
