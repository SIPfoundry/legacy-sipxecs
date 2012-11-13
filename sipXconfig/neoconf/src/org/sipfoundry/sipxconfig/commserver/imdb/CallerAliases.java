/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ANONYMOUS;
import static org.sipfoundry.commons.mongo.MongoConstants.CALLERALIAS;
import static org.sipfoundry.commons.mongo.MongoConstants.CID_PREFIX;
import static org.sipfoundry.commons.mongo.MongoConstants.IGNORE_USER_CID;
import static org.sipfoundry.commons.mongo.MongoConstants.KEEP_DIGITS;
import static org.sipfoundry.commons.mongo.MongoConstants.TRANSFORM_EXT;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;

import com.mongodb.DBObject;

public class CallerAliases extends AbstractDataSetGenerator {

    @Override
    protected DataSet getType() {
        return DataSet.CALLER_ALIAS;
    }

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            if (StringUtils.isNotBlank(user.getSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER))) {
                top.put(CALLERALIAS, SipUri.format(user.getDisplayName(),
                        user.getSettingValue(UserCallerAliasInfo.EXTERNAL_NUMBER), getSipDomain()));
            } else {
                removeField(top, CALLERALIAS);
            }
            return true;
        } else if (entity instanceof Gateway) {
            Gateway gateway = (Gateway) entity;
            final GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
            top.put(UID, Gateway.UID);
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
            return true;
        }
        return false;
    }
}
