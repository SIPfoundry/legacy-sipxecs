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

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.setting.Setting;

public class OpenAcdLine extends OpenAcdExtension {
    public static final Integer FS = new Integer(1);
    public static final Integer ACD = new Integer(2);
    public static final Integer AGENT = new Integer(3);
    public static final String Q = "queue=";
    public static final String BRAND = "brand=";
    public static final String DESTINATION_NUMBER = "destination_number";
    public static final String ALLOW_VOICEMAIL = "allow_voicemail=";
    public static final String ERLANG_ANSWER = "erlang_answer=false";
    public static final String EMPTY_STRING = "";
    public static final String OPEN_ACD = "openacd@";
    public static final String PATH_CLIENT_ID = "openacd-line/client-identity";
    public static final String PATH_CLIENT_NAME = "openacd-line/client-name";
    public static final String PATH_QUEUE_NAME = "openacd-line/queue-name";

    public static List<FreeswitchAction> getDefaultActions(Location location) {
        List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
        actions.add(createAction(FreeswitchAction.PredefinedAction.answer.toString(), null));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), "domain_name=$${domain}"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), Q));
        actions.add(createAction(FreeswitchAction.PredefinedAction.set.toString(), "allow_voicemail=true"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.erlang_sendmsg.toString(),
                "freeswitch_media_manager  " + OPEN_ACD + location.getFqdn() + " inivr ${uuid}"));
        actions.add(createAction(FreeswitchAction.PredefinedAction.playback.toString(), EMPTY_STRING));
        actions.add(createAction(FreeswitchAction.PredefinedAction.erlang.toString(),
                "freeswitch_media_manager:!  " + OPEN_ACD + location.getFqdn()));
        return actions;
    }

    public static FreeswitchAction createVoicemailAction(boolean allow) {
        return allow ? createAction(FreeswitchAction.PredefinedAction.set.toString(), ALLOW_VOICEMAIL + allow)
                : null;
    }

    public static FreeswitchAction createAnswerAction(Integer type) {
        if (type == FS) {
            return createAction(FreeswitchAction.PredefinedAction.answer.toString(), null);
        } else if (type == AGENT) {
            return createAction(FreeswitchAction.PredefinedAction.set.toString(), ERLANG_ANSWER);
        } else {
            return null;
        }
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(OpenAcdContext.DID, getDid());
        props.put(OpenAcdContext.CLIENT_NAME, getSettingValue(PATH_CLIENT_NAME));
        props.put(OpenAcdContext.CLIENT_ID, getSettingValue(PATH_CLIENT_ID));
        props.put(OpenAcdContext.Q_NAME, getSettingValue(PATH_QUEUE_NAME));
        props.put(OpenAcdContext.LINE_NAME, getName());
        props.putAll(super.getMongoProperties(domain));
        return props;
    }

    public FreeswitchAction createQueueAction(OpenAcdQueue queue) {
        setSettingValue(PATH_QUEUE_NAME, queue.getName());
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), Q + queue.getName());
    }

    public FreeswitchAction createClientAction(OpenAcdClient client) {
        setSettingValue(PATH_CLIENT_NAME, client.getName());
        setSettingValue(PATH_CLIENT_ID, client.getIdentity());
        return createAction(FreeswitchAction.PredefinedAction.set.toString(), BRAND + client.getIdentity());
    }

    public static FreeswitchAction createPlaybackAction(String path) {
        return createAction(FreeswitchAction.PredefinedAction.playback.toString(), path);
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(this);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("openacd/openacdline.xml");
    }
}
