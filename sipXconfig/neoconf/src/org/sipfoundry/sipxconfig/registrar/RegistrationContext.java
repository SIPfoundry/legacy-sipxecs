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

    public abstract List<RegistrationItem> getRegistrations(Integer start, Integer limit);

    public abstract List<RegistrationItem> getRegistrationsByUser(User user);

    public abstract List<RegistrationItem> getRegistrationsByUser(User user, Integer start, Integer limit);

    public abstract List<RegistrationItem> getRegistrationsByLineId(String uid);

    public abstract List<RegistrationItem> getRegistrationsByMac(String mac);

    public abstract List<RegistrationItem> getRegistrationsByIp(String ip);

    public abstract List<RegistrationItem> getRegistrationsByServer(String server);

    public abstract List<RegistrationItem> getRegistrationsByCallId(String callId);

    public abstract void dropRegistrationsByUser(User user);

    public abstract void dropRegistrationsByMac(String mac);

    public abstract void dropRegistrationsByIp(String ip);

    public abstract void dropRegistrationsByServer(String server);

    public abstract void dropRegistrationsByCallId(String callId);

    @Deprecated
    DBCursor getMongoDbCursorRegistrationsByLineId(String uid);

    @Deprecated
    DBCursor getMongoDbCursorRegistrationsByMac(String mac);

    @Deprecated
    DBCursor getMongoDbCursorRegistrationsByIp(String ip);
}
