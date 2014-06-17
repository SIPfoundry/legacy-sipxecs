/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.hz;

import java.util.concurrent.Callable;

import com.hazelcast.core.HazelcastInstance;
import com.hazelcast.core.ITopic;

public class HzPublisherTask implements Callable<Object> {

    private Object m_event;
    private String m_structureName;
    private HazelcastInstance m_instance;

    public HzPublisherTask(HazelcastInstance instance, Object event, String structureName) {
        m_instance = instance;
        m_event = event;
        m_structureName = structureName;
    }

    @Override
    public Boolean call() {
        try {
            ITopic<Object> topic = m_instance.getTopic(m_structureName);
            topic.publish(m_event);
        } catch (Exception ex) {
            return false;
        }
        return true;
    }

}
