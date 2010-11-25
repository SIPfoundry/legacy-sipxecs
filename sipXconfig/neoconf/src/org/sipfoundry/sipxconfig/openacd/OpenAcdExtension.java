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
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;

public class OpenAcdExtension extends FreeswitchExtension {
    public static final String Q = "queue=";
    public static final String DESTINATION_NUMBER = "destination_number";
    public static final String ALLOW_VOICEMAIL = "allow_voicemail=";
    public static final String DESTINATION_NUMBER_PATTERN = "^%s$";
    public static final String EMPTY_STRING = "";

    public FreeswitchCondition getLineCondition() {
        if (getConditions() == null) {
            return null;
        }
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                return condition;
            }
        }
        return null;
    }

    public boolean isOpenAcdLine() {
        if (getConditions() == null) {
            return false;
        }
        if (getLineCondition() == null) {
            return false;
        }
        return true;
    }

    public String getLineNumber() {
        if (isOpenAcdLine()) {
            return getLineCondition().getLineNumber();
        }
        return null;
    }

    public List<FreeswitchAction> getLineActions() {
        if (!isOpenAcdLine()) {
            return null;
        }
        List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                for (FreeswitchAction action : condition.getActions()) {
                    actions.add(action);
                }
            }
        }
        return actions;
    }

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

    private static FreeswitchAction createAction(String application, String data) {
        FreeswitchAction action = new FreeswitchAction();
        action.setApplication(application);
        action.setData(data);
        return action;
    }

    public static FreeswitchCondition createLineCondition() {
        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField(OpenAcdExtension.DESTINATION_NUMBER);
        condition.setExpression(EMPTY_STRING);
        return condition;
    }

    public static FreeswitchAction createAnswerAction(boolean answer) {
        if (!answer) {
            return null;
        }
        return createAction(FreeswitchAction.PredefinedAction.answer.toString(), null);
    }

    public static FreeswitchAction createVoicemailAction(boolean allow) {
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), ALLOW_VOICEMAIL + allow);
    }

    public static FreeswitchAction createQueueAction(String queue) {
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), Q + queue);
    }

}
