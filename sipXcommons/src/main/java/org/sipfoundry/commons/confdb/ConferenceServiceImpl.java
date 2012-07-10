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

import static org.sipfoundry.commons.mongo.MongoConstants.CONF_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.CONF_OWNER;

import java.util.List;

import org.apache.log4j.Logger;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.springframework.data.mongodb.core.query.Criteria;
import org.springframework.data.mongodb.core.query.Query;

public class ConferenceServiceImpl implements ConferenceService {
    private MongoTemplate m_template;
    private static final String CONF_COLLECTION = "entity";
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");
    @Override
    public Conference getConference(String name) {
        return m_template.findOne(new Query(
                Criteria.where(CONF_NAME).is(name)), Conference.class, CONF_COLLECTION);
    }

    public List<Conference> getOwnedConferences(String username) {
        return m_template.find(new Query(Criteria.
                where(CONF_OWNER).is(username).
                and(CONF_ENABLED).is(true)), Conference.class, CONF_COLLECTION);
    }

    public void setTemplate(MongoTemplate template) {
        m_template = template;
    }

}
