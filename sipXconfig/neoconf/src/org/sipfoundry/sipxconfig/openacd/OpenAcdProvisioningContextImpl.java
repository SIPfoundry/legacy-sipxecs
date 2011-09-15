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

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.WriteResult;

import org.sipfoundry.commons.mongo.MongoAccessController;
import org.sipfoundry.sipxconfig.common.UserChangeEvent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

import static org.apache.commons.beanutils.BeanUtils.getSimpleProperty;
import static org.apache.commons.lang.StringUtils.EMPTY;

public class OpenAcdProvisioningContextImpl implements OpenAcdProvisioningContext, ApplicationListener {
    enum Command {
        ADD {
            public String toString() {
                return "ADD";
            }
        },

        DELETE {
            public String toString() {
                return "DELETE";
            }
        },

        UPDATE {
            public String toString() {
                return "UPDATE";
            }
        },

        CONFIGURE {
            public String toString() {
                return "CONFIGURE";
            }
        }
    }

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
                updateObjects(list);
            }
        }
    }

    @Override
    public void deleteObjects(List< ? extends OpenAcdConfigObject> openAcdObjects) {
        storeCommand(createCommand(Command.DELETE, openAcdObjects));
    }

    @Override
    public void addObjects(List< ? extends OpenAcdConfigObject> openAcdObjects) {
        storeCommand(createCommand(Command.ADD, openAcdObjects));
    }

    @Override
    public void updateObjects(List< ? extends OpenAcdConfigObject> openAcdObjects) {
        storeCommand(createCommand(Command.UPDATE, openAcdObjects));
    }

    @Override
    public void configure(List< ? extends OpenAcdConfigObject> openAcdObjects) {
        storeCommand(createCommand(Command.CONFIGURE, openAcdObjects));
    }

    protected void storeCommand(BasicDBObject command) {
        try {
            DB openAcdDb = MongoAccessController.INSTANCE.getDatabase("openacd");
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

    private static BasicDBObject createCommand(Command openAcdCommand,
            List< ? extends OpenAcdConfigObject> openAcdObjects) {
        BasicDBObject command = new BasicDBObject();
        command.put("command", openAcdCommand.toString());
        command.put("count", openAcdObjects.size());
        command.put("objects", getObjects(openAcdObjects));
        return command;
    }

    private static List<BasicDBObject> getObjects(List< ? extends OpenAcdConfigObject> openAcdObjects) {
        List<BasicDBObject> objects = new LinkedList<BasicDBObject>();
        for (OpenAcdConfigObject openAcdObject : openAcdObjects) {
            BasicDBObject object = new BasicDBObject();
            object.put("type", openAcdObject.getType());
            for (String property : openAcdObject.getProperties()) {
                object.put(property, getDataValue(openAcdObject, property));
            }
            if (openAcdObject instanceof EnhancedOpenAcdConfigObject) {
                object.put("additionalObjects", ((EnhancedOpenAcdConfigObject) openAcdObject).getAdditionalObjects());
            }
            objects.add(object);
        }
        return objects;
    }

    private static String getDataValue(Object bean, String name) {
        try {
            return getSimpleProperty(bean, name);
        } catch (IllegalAccessException e) {
            return EMPTY;
        } catch (InvocationTargetException e) {
            return EMPTY;
        } catch (NoSuchMethodException e) {
            return EMPTY;
        }
    }

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
