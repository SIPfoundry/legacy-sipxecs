package org.sipfoundry.openfire.plugin.job.vcard;

import java.io.StringReader;
import java.security.MessageDigest;

import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.jivesoftware.openfire.PresenceManager;
import org.jivesoftware.openfire.XMPPServer;
import org.jivesoftware.openfire.XMPPServerInfo;
import org.jivesoftware.openfire.handler.PresenceUpdateHandler;
import org.jivesoftware.openfire.provider.VCardProvider;
import org.jivesoftware.openfire.user.User;
import org.jivesoftware.openfire.user.UserManager;
import org.jivesoftware.openfire.vcard.VCardEventDispatcher;
import org.jivesoftware.openfire.vcard.VCardManager;
import org.sipfoundry.openfire.sync.job.Job;
import org.xmpp.packet.Presence;

import com.mongodb.DBObject;

public class VcardUpdateJob implements Job {
    private static Logger logger = Logger.getLogger(VcardUpdateJob.class);
    /**
     *
     */
    private static final long serialVersionUID = 1L;
    protected final String userImName;

    public VcardUpdateJob(String userImName, DBObject dbObj) {
        this.userImName = userImName;
    }

    @Override
    public void process() {
        logger.debug("start processing " + toString());
        VCardProvider provider = VCardManager.getProvider();
        Element userVCard = provider.loadVCard(userImName);
        try {
            logger.debug("dispatch user update vcard");
            VCardEventDispatcher.dispatchVCardUpdated(userImName, userVCard);

            updateAvatar(userImName, userVCard);
        } catch (Exception e) {
            logger.error(e.getMessage(), e);
            for (StackTraceElement el : e.getStackTrace()) {
                logger.error(el.toString());
            }
        }
        logger.debug("end processing " + toString());
    }

    protected static void updateAvatar(String username, Element vCard) throws Exception {
        if (vCard.element("PHOTO") == null) {
            return;
        }
        Element binValElement = vCard.element("PHOTO").element("BINVAL");
        if (binValElement == null) {
            return;
        }
        String avatarStr = binValElement.getText();
        XMPPServer server = XMPPServer.getInstance();
        XMPPServerInfo info = server.getServerInfo();
        String aor = username + "@" + info.getXMPPDomain();
        String itemId = getItemId(avatarStr.getBytes());
        Presence presenceAvatar = createPresenceAvatar(aor, itemId);
        PresenceUpdateHandler puh = server.getPresenceUpdateHandler();
        logger.debug("processing " + presenceAvatar.toXML());
        puh.process(presenceAvatar);
    }

    private static String getItemId(byte[] avatarBytes) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        md.update(avatarBytes);
        byte[] digest = md.digest();
        return byteArrayToString(digest);
    }

    private static String byteArrayToString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            sb.append(String.format("%02x", bytes[i]));
        }
        return sb.toString();
    }

    private static Presence createPresenceAvatar(String aor, String itemId) throws Exception {
        StringBuilder builder = new StringBuilder("<presence>");
        builder.append("<priority>1</priority>");
        builder.append("<c xmlns='http://jabber.org/protocol/caps' node='http://pidgin.im/' hash='sha-1' ver='I22W7CegORwdbnu0ZiQwGpxr0Go='/>");
        builder.append("<x xmlns='vcard-temp:x:update'>");
        builder.append("<photo>");
        builder.append(itemId);
        builder.append("</photo></x></presence>");

        String xmlstr = builder.toString();
        SAXReader sreader = new SAXReader();

        Document avatarDoc = sreader.read(new StringReader(xmlstr));
        Element rootElement = avatarDoc.getRootElement();

        Presence avatar = new Presence(rootElement);

        avatar.setFrom(aor);

        XMPPServer.getInstance();
        UserManager um = XMPPServer.getInstance().getUserManager();
        User me = um.getUser(aor);

        PresenceManager pm = XMPPServer.getInstance().getPresenceManager();
        Presence mypresence = pm.getPresence(me);

        avatar.setType(mypresence.getType());
        avatar.setShow(mypresence.getShow());
        avatar.setStatus(mypresence.getStatus());
        avatar.setID(aor + "_presenceAvatar");
        return avatar;
    }

    @Override
    public String toString() {
        return "VcardUpdateJob [userImName=" + userImName + "]";
    }

}
