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

import static org.apache.commons.beanutils.BeanUtils.getSimpleProperty;
import static org.apache.commons.lang.StringUtils.EMPTY;

import java.lang.reflect.InvocationTargetException;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.WriteResult;

public class OpenAcdProvisioningContextImpl implements OpenAcdProvisioningContext,
        BeanFactoryAware {
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

    private MongoTemplate m_db;
    private ListableBeanFactory m_beanFactory;


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

    public void resync() {
        Map<String, OpenAcdConfigObjectProvider> beanMap = m_beanFactory
                .getBeansOfType(OpenAcdConfigObjectProvider.class);
        for (OpenAcdConfigObjectProvider bean : beanMap.values()) {
            for (OpenAcdConfigObject configObj : bean.getConfigObjects()) {
                if (!configObj.isConfigCommand()) {
                    addObjects(Collections.singletonList(configObj));
                } else {
                    configure(Collections.singletonList(configObj));
                }
            }
        }
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

}
