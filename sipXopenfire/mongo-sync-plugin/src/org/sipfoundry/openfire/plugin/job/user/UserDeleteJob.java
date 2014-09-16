/**
 *
 *
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
package org.sipfoundry.openfire.plugin.job.user;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.sync.job.Job;


public class UserDeleteJob implements Job {
    private static Logger logger = Logger.getLogger(UserDeleteJob.class);

    private static final long serialVersionUID = 1L;

    private final String userImName;

    public UserDeleteJob(String userImName) {
        this.userImName = userImName;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());

        UserShared.removeUser(userImName);
        logger.debug("end processing " + toString());
    }

    @Override
    public String toString() {
        return "UserDeleteJob [userImName=" + userImName + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((userImName == null) ? 0 : userImName.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        UserDeleteJob other = (UserDeleteJob) obj;
        if (userImName == null) {
            if (other.userImName != null) {
                return false;
            }
        } else if (!userImName.equals(other.userImName)) {
            return false;
        }
        return true;
    }
}
