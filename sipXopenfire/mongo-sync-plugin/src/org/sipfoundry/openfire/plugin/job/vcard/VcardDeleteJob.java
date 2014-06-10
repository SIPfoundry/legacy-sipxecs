package org.sipfoundry.openfire.plugin.job.vcard;

import org.apache.log4j.Logger;
import org.dom4j.Element;
import org.jivesoftware.openfire.provider.VCardProvider;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.sipfoundry.openfire.provider.CacheHolder;

/**
 * We are not actually deleting the avatar, but setting it to
 * the default image. However this job is processed in the event of
 * a DELETE operation from OpLog
 */
public class VcardDeleteJob extends VcardUpdateJob {
    private static Logger logger = Logger.getLogger(VcardDeleteJob.class);
    /**
     *
     */
    private static final long serialVersionUID = 1L;

    public VcardDeleteJob(String userImName) {
        super(userImName, null);
    }

    @Override
    public void process() {
        try {
            logger.info("Processing vcard delete job for " + toString());
            VCardProvider provider = VCardManager.getProvider();
            Element vcard = provider.loadVCard(userImName);

            logger.debug("delete vcard!");
            VCardManager.getInstance().setVCard(userImName, vcard);
            CacheHolder.removeAvatarByImId(userImName);

            updateAvatar(userImName, vcard);
        } catch (Exception e) {
            logger.error(e);
        }
    }

    @Override
    public String toString() {
        return "VcardDeleteJob [userImName=" + userImName + ", oldMd5=" + oldMd5 + ", newMd5=" + newMd5 + "]";
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 1;
        result = prime * result + ((newMd5 == null) ? 0 : newMd5.hashCode());
        result = prime * result + ((oldMd5 == null) ? 0 : oldMd5.hashCode());
        result = prime * result + ((userImName == null) ? 0 : userImName.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        VcardDeleteJob other = (VcardDeleteJob) obj;
        if (newMd5 == null) {
            if (other.newMd5 != null)
                return false;
        } else if (!newMd5.equals(other.newMd5))
            return false;
        if (oldMd5 == null) {
            if (other.oldMd5 != null)
                return false;
        } else if (!oldMd5.equals(other.oldMd5))
            return false;
        if (userImName == null) {
            if (other.userImName != null)
                return false;
        } else if (!userImName.equals(other.userImName))
            return false;
        return true;
    }

}
