/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.HibernateEntityChangeProvider;

/**
 * This interface handles operations like building and saving ConfigChange
 * objects
 */
public interface SystemAuditManager extends HibernateEntityChangeProvider {

    /**
     * This method only handles UserProfile saves, which don't go through
     * hibernate but are persisted in mongo db
     */
    public void auditUserProfile(User user);

}
