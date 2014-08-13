package org.sipfoundry.openfire.plugin.job.group;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.sync.job.Job;

public class GroupDeleteJob implements Job {
    private static Logger logger = Logger.getLogger(GroupDeleteJob.class);

    private static final long serialVersionUID = 1L;

    private final String groupName;

    public GroupDeleteJob(String groupName) {
        this.groupName = groupName;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());

        GroupShared.removeGroup(groupName);
        logger.debug("finished processing " + toString());
    }

    @Override
    public String toString() {
        return "GroupDeleteJob [groupName=" + groupName + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((groupName == null) ? 0 : groupName.hashCode());
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
        GroupDeleteJob other = (GroupDeleteJob) obj;
        if (groupName == null) {
            if (other.groupName != null) {
                return false;
            }
        } else if (!groupName.equals(other.groupName)) {
            return false;
        }
        return true;
    }
}
