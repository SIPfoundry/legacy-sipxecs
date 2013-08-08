/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.mongo;

public interface MongoReplSetManager {

    public boolean isInProgress();

    public MongoMeta getMeta();

    /** only makes sense for local database API */
    public String addFirstLocalDatabase(String server);

    /**
     * Only to be used on last local database, otherwise just removeDatabase
     */
    public String removeLastLocalDatabase(String hostPort);

    public String removeLastLocalArbiter(String hostPort);

    public String addDatabase(String primary, String server);

    public String addArbiter(String primary, String server);

    public String removeDatabase(String primary, String server);

    public String removeArbiter(String primary, String server);

    public String addLocalDatabase(String primary, String hostPort);

    public String addLocalArbiter(String primary, String hostPort);

    public String removeLocalDatabase(String primary, String hostPort);

    public String removeLocalArbiter(String primary, String hostPort);
    
    public String takeAction(String primary, String server, String action);

    public String getLastConfigError();
}
