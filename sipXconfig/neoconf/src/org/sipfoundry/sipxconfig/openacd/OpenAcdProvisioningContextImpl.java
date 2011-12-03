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


import java.util.ArrayList;

import org.sipfoundry.sipxconfig.common.UserChangeEvent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationEvent;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.WriteResult;

public class OpenAcdProvisioningContextImpl implements OpenAcdProvisioningContext,
        BeanFactoryAware {
    enum Command {
        RESYNC {
            public String toString() {
                return "RESYNC";
            }
        }
    }

    private MongoTemplate m_db;
    private ListableBeanFactory m_beanFactory;
    private OpenAcdContext m_openAcdContext;

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof UserChangeEvent) {
            UserChangeEvent userEvent = (UserChangeEvent) event;
            OpenAcdAgent agent = m_openAcdContext.getAgentByUserId(userEvent.getUserId());
            if (agent != null) {
                agent.setOldName(userEvent.getOldUserName());
                agent.getUser().setUserName(userEvent.getUserName());
                agent.getUser().setFirstName(userEvent.getFirstName());
                agent.getUser().setLastName(userEvent.getLastName());

                ArrayList<OpenAcdAgent> list = new ArrayList<OpenAcdAgent>();
                list.add(agent);
                m_openAcdContext.saveAgent(agent);
            }
        }
    }

    protected void storeCommand(BasicDBObject command) {
        try {
            DB openAcdDb = m_db.getDb();
            openAcdDb.requestStart();
            DBCollection commandsCollection = openAcdDb.getCollection("commands");
            WriteResult result = commandsCollection.insert(command);
            String error = result.getError();
            if (error != null) {
                throw new UserException("&msg.mongo.write.error", error);
            }
            openAcdDb.requestDone();
        } catch (Exception ex) {
            throw new UserException("&msg.cannot.connect");
        }
    }

    private static BasicDBObject createCommand(Command openAcdCommand) {
        BasicDBObject command = new BasicDBObject();
        command.put("command", openAcdCommand.toString());
        return command;
    }

    public void resync() {
        storeCommand(createCommand(Command.RESYNC));
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public MongoTemplate getDb() {
        return m_db;
    }

    public void setDb(MongoTemplate db) {
        m_db = db;
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
