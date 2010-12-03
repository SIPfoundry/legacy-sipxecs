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

import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;

public class OpenAcdExtension extends FreeswitchExtension {
    public static final String DESTINATION_NUMBER = "destination_number";
    public static final String DESTINATION_NUMBER_PATTERN = "^%s$";
    public static final String EMPTY_STRING = "";

    /**
     * We call this condition the (first, because they can be many) condition that has
     * destination_number as a field
     */
    public FreeswitchCondition getNumberCondition() {
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

    public String getExtension() {
        if (getNumberCondition() != null) {
            return getNumberCondition().getExtension();
        }
        return null;
    }

    public List<FreeswitchAction> getLineActions() {
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

    protected static FreeswitchAction createAction(String application, String data) {
        FreeswitchAction action = new FreeswitchAction();
        action.setApplication(application);
        action.setData(data);
        return action;
    }

    public static FreeswitchCondition createLineCondition() {
        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField(OpenAcdLine.DESTINATION_NUMBER);
        condition.setExpression(EMPTY_STRING);
        return condition;
    }

    public static FreeswitchAction createAnswerAction(boolean answer) {
        if (!answer) {
            return null;
        }
        return createAction(FreeswitchAction.PredefinedAction.answer.toString(), null);
    }
}
