/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.mongo.MongoAccessController;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);
    private static final String DB_NAME = "node";
    private static final String DB_COLLECTION_NAME = "registrar";
    private static final String UNABLE_OPEN_MONGO = "Unable to open mongo";
    private LocationsManager m_locationsManager;

    private DB getDatabase(String dbName) throws Exception {
        try {
            return MongoAccessController.INSTANCE.getDatabase(dbName);
        } catch (Exception e) {
            LOG.error(UNABLE_OPEN_MONGO);
            throw (e);
        }
    }

    /**
     * @see org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext#getRegistrations()
     */
    public List<RegistrationItem> getRegistrations() {
        try {
            Location primaryProxyLocation = m_locationsManager.getLocationByBundle("primarySipRouterBundle");
            if (primaryProxyLocation == null) {
                LOG.error("No primary proxy found.");
                return Collections.emptyList();
            }
            DB datasetDb = getDatabase(DB_NAME);
            DBCollection registrarCollection = datasetDb.getCollection(DB_COLLECTION_NAME);
            DBCursor cursor = registrarCollection.find();
            List<RegistrationItem> items = new ArrayList<RegistrationItem>(cursor.size());
            while (cursor.hasNext()) {
                DBObject registration = cursor.next();
                if ((Boolean) registration.get("expired")) {
                    continue;
                }
                RegistrationItem item = new RegistrationItem();
                item.setContact((String) registration.get("contact"));
                item.setPrimary((String) registration.get("primary"));
                item.setExpires((Integer) registration.get("expirationTime"));
                item.setUri((String) registration.get("uri"));
                item.setInstrument((String) registration.get("instrument"));
                items.add(item);
            }
            return items;
        } catch (Exception e) {
            // we are handling this separately - server returns FileNotFound even if everything is
            // OK but we have no registrations present
            LOG.warn("Cannot retrieve registrations.", e);
            return Collections.emptyList();
        }
    }

    public List<RegistrationItem> getRegistrationsByUser(User user) {
        return getRegistrationsByUser(getRegistrations(), user);
    }

    List<RegistrationItem> getRegistrationsByUser(List<RegistrationItem> registrations, User user) {
        List<RegistrationItem> result = new ArrayList<RegistrationItem>();
        for (RegistrationItem registration : registrations) {
            if (SipUri.extractUser(registration.getUri()).equals(user.getUserName())) {
                result.add(registration);
            }
        }
        return result;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

}
