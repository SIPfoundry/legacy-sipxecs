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
package org.sipfoundry.commons.confdb;

import static org.sipfoundry.commons.mongo.MongoConstants.CONF_AUTORECORD;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_DESCRIPTION;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_EXT;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_MEMBERS_ONLY;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_MODERATED;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_OWNER;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_PIN;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_PUBLIC;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_URI;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.springframework.core.convert.converter.Converter;

import com.mongodb.DBObject;

public class ConfReadConverter implements Converter<DBObject, Conference> {
    @Override
    public Conference convert(DBObject source) {
        Conference conf = new Conference();
        conf.setId((String) source.get(MongoConstants.ID));
        conf.setConfName((String) source.get(CONF_NAME));
        conf.setConfDescription((String) source.get(CONF_DESCRIPTION));
        conf.setConfOwner((String) source.get(CONF_OWNER));
        conf.setModerated(Boolean.valueOf(!StringUtils.equals((String)source.get(CONF_MODERATED), "0")));
        conf.setPublic(Boolean.valueOf(!StringUtils.equals((String)source.get(CONF_PUBLIC), "0")));
        conf.setMembersOnly(Boolean.valueOf(!StringUtils.equals((String)source.get(CONF_MEMBERS_ONLY), "0")));
        conf.setExtension((String) source.get(CONF_EXT));
        conf.setPin((String) source.get(CONF_PIN));
        conf.setAutoRecord((Boolean)source.get(CONF_AUTORECORD));
        conf.setUri((String)source.get(CONF_URI));
        return conf;
    }

}
