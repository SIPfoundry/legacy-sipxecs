/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;

import com.mongodb.DBCursor;

public interface RegistrationContext {

    public abstract List<RegistrationItem> getRegistrations();

    public abstract List<RegistrationItem> getRegistrationsByUser(User user);

    DBCursor getMongoDbCursorRegistrationsByLineId(String uid);

    DBCursor getRegistrationsByMac(String mac);

    DBCursor getRegistrationsByIp(String ip);

    List<RegistrationItem> getRegistrationsByLineId(String line);
}
