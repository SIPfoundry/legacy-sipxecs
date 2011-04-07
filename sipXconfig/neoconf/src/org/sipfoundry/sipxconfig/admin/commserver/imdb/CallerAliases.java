/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;

public class CallerAliases extends DataSetGenerator {
    public static final String CALLERALIAS = "clrid";
    public static final String IGNORE_USER_CID = "ignorecid";
    public static final String CID_PREFIX = "pfix";
    public static final String KEEP_DIGITS = "kpdgts";
    public static final String TRANSFORM_EXT = "trnsfrmext";
    public static final String ANONYMOUS = "blkcid";
    public static final String DISPLAY_NAME = "name";
    public static final String URL_PARAMS = "url";

    @Override
    protected DataSet getType() {
        return DataSet.CALLER_ALIAS;
    }

    @Override
    public void generate(Replicable entity) {
        if (entity instanceof User) {
            User user = (User) entity;
            DBObject top = findOrCreate(user);
            if (StringUtils.isNotBlank(user.getSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER))) {
                top.put(CALLERALIAS, SipUri.format(user.getDisplayName(),
                        user.getSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER), getSipDomain()));
            } else {
                top.put(CALLERALIAS, StringUtils.EMPTY);
            }
            getDbCollection().save(top);
        } else if (entity instanceof Gateway) {
            Gateway gateway = (Gateway) entity;
            final GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
            DBObject top = findOrCreate(gateway);
            top.put("uid", Gateway.UID);
            if (StringUtils.isNotBlank(gatewayInfo.getDefaultCallerAlias())) {
                top.put(CALLERALIAS, SipUri.fixWithDisplayName(gatewayInfo.getDefaultCallerAlias(),
                        gatewayInfo.getDisplayName(), gatewayInfo.getUrlParameters(), getSipDomain()));
            } else {
                top.put(CALLERALIAS, StringUtils.EMPTY);
            }
            top.put(IGNORE_USER_CID, gatewayInfo.isIgnoreUserInfo());
            top.put(CID_PREFIX, gatewayInfo.getAddPrefix());
            top.put(KEEP_DIGITS, gatewayInfo.getKeepDigits());
            top.put(TRANSFORM_EXT, gatewayInfo.isTransformUserExtension());
            top.put(ANONYMOUS, gatewayInfo.isAnonymous());
            getDbCollection().save(top);
        }
    }
}
