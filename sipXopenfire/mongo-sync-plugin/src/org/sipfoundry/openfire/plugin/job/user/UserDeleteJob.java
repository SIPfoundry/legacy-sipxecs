package org.sipfoundry.openfire.plugin.job.user;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.plugin.job.Job;


public class UserDeleteJob implements Job {
    private static Logger logger = Logger.getLogger(UserDeleteJob.class);

    private static final long serialVersionUID = 1L;

    private final String userImName;

    public UserDeleteJob(String userImName) {
        this.userImName = userImName;
    }

    @Override
    public void process() {
        logger.debug("processing " + toString());

        UserShared.removeUser(userImName);
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
