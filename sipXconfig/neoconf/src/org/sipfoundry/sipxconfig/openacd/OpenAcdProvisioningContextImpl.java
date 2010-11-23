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

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.Mongo;

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
        }
    }

    private String m_host = "localhost";
    private int m_port = 27017;

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

    public void setHost(String host) {
        m_host = host;
    }

    public void setPort(int port) {
        m_port = port;
    }

    private void storeCommand(BasicDBObject command) {
        try {
            Mongo mongoInstance = new Mongo(m_host, m_port);
            DB openAcdDb = mongoInstance.getDB("openacd");
            DBCollection commandsCollection = openAcdDb.getCollection("commands");
            commandsCollection.insert(command);
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
