package org.sipfoundry.sipximbot;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.jivesoftware.smack.ConnectionConfiguration;
import org.jivesoftware.smack.PacketListener;
import org.jivesoftware.smack.Roster;
import org.jivesoftware.smack.RosterEntry;
import org.jivesoftware.smack.RosterListener;
import org.jivesoftware.smack.XMPPConnection;
import org.jivesoftware.smack.XMPPException;
import org.jivesoftware.smack.filter.PacketFilter;
import org.jivesoftware.smack.filter.PacketTypeFilter;
import org.jivesoftware.smack.packet.Packet;
import org.jivesoftware.smack.packet.PacketExtension;
import org.jivesoftware.smack.packet.Presence;
import org.jivesoftware.smack.util.DNSUtil;
import org.jivesoftware.smack.util.dns.JavaxResolver;
import org.jivesoftware.smackx.packet.VCard;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.sipximbot.IMUser.UserPresence;

public class IMBot {
    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");

    // in milliseconds
    private static final long RETRY_INTERVAL = 30 * 1000;
    private static final long MAX_RETRIES = 10;

    private static Roster m_roster;

    // key is Jabber Id, value is IMUser
    private static Map<String, IMUser> m_ChatsMap = Collections.synchronizedMap(new HashMap<String, IMUser>());

    private static class IMClientThread extends Thread {
        private static XMPPConnection m_con;
        private static Localizer m_localizer;

        public IMClientThread() {
        }

        /*
         * IMbot client computes the SHA1 hash of the avatar image data itself. Include this hash
         * in the user's presence information as the XML character data of the <photo/> child of
         * an <x/> element qualified by the 'vcard-temp:x:update' namespace
         */
        private class AvatarUpdateExtension implements PacketExtension {
            private String photoHash;

            public void setPhotoHash(String hash) {
                photoHash = hash;
            }

            @Override
            public String getElementName() {
                return "x";
            }

            @Override
            public String getNamespace() {
                return "vcard-temp:x:update";
            }

            @Override
            public String toXML() {
                StringBuffer buf = new StringBuffer();
                buf.append("<").append(getElementName()).append(" xmlns=\"").append(getNamespace()).append("\">");
                buf.append("<photo>");
                buf.append(photoHash);
                buf.append("</photo>");
                buf.append("</").append(getElementName()).append(">");
                return buf.toString();
            }
        }

        private void updateAvatar() {

            boolean savingAvatar = false;
            VCard vCard = new VCard();
            try {
                vCard.load(m_con);
                URL url;
                try {
                    url = new URL("file://" + System.getProperty("var.dir", "/var/sipxdata")
                        + "/sipximbot/image/avatar.jpg");
                    vCard.setAvatar(url);
                    savingAvatar = true;
                    vCard.save(m_con);
                    updateAvatarPresence(vCard);
                } catch (MalformedURLException e) {
                    LOG.error("Malformed URL in updateAvatar ");
                }

            } catch (XMPPException xmppe) {
                if (savingAvatar) {
                    // google talk doesn't like us saving avatars immediately after login!
                    try {
                        sleep(10000);
                    } catch (InterruptedException e) {

                    }
                    try {
                        vCard.save(m_con);
                        updateAvatarPresence(vCard);
                    } catch (XMPPException e) {
                        LOG.error("could not update Avatar " + e.getMessage());
                    }
                }
            }
        }

        private void updateAvatarPresence(VCard vCard) {
            Presence aPresence = new Presence(Presence.Type.available);
            AvatarUpdateExtension AvatarExt = new AvatarUpdateExtension();
            AvatarExt.setPhotoHash(vCard.getAvatarHash());
            aPresence.addExtension(AvatarExt);
            aPresence.setStatus(m_localizer.localize("cmd_help"));
            aPresence.setFrom(ImbotConfiguration.get().getMyAsstAcct());
            m_con.sendPacket(aPresence);
        }

        private static boolean connectToXMPPServer() {

            ImbotConfiguration config = ImbotConfiguration.get();
            ConnectionConfiguration conf = new ConnectionConfiguration(config.getOpenfireHost());
            // disable SASL to cope with cases where XMPP domain != FQDN (XX-7293)
            conf.setSASLAuthenticationEnabled(false);
            Roster.setDefaultSubscriptionMode(Roster.SubscriptionMode.manual);
            m_con = new XMPPConnection(conf);

            for (int i = 0; i < MAX_RETRIES; i++) {
                try {
                    m_con.connect();
                    // only keep user part and ditch the @domain part if present
                    String username = config.getMyAsstAcct().split("@")[0];
                    m_con.login(username, config.getMyAsstPswd());
                    return true;
                } catch (Exception e) {
                    // typically get this exception if server is unreachable or login info is
                    // wrong
                    // only thing do it is periodically retry just like any other IM client would

                    LOG.error("Could not login to XMPP server " + e.getMessage());
                }
                try {
                    LOG.info(String.format(
                        "Waiting %d seconds before attempting another connection to XMPP server.",
                        RETRY_INTERVAL / 1000));
                    sleep(RETRY_INTERVAL);
                } catch (InterruptedException e) {
                    return false;
                }
            }
            throw new RuntimeException("Could not establish connection to XMPP server after " + MAX_RETRIES
                + " attempts");
        }

        public static void AddToRoster(User user) {
            Presence presPacket = new Presence(Presence.Type.subscribe);
            presPacket.setFrom(ImbotConfiguration.get().getMyAsstAcct());

            if (user.getJid() != null) {
                presPacket.setTo(user.getJid());
                m_con.sendPacket(presPacket);
            }

            if (user.getAltJid() != null) {
                presPacket.setTo(user.getAltJid());
                m_con.sendPacket(presPacket);
            }
        }

        @Override
        public void run() {

            boolean running = true;

            if (!connectToXMPPServer()) {
                return;
            }

            m_localizer = new Localizer();

            updateAvatar();
            m_roster = m_con.getRoster();

            class PresenceListener implements PacketListener {

                @Override
                public void processPacket(Packet packet) {
                    Presence presence = (Presence) packet;
                    Presence presPacket = null;

                    switch (presence.getType()) {
                    case subscribe:
                        String jid = presence.getFrom();
                        if (jid.indexOf('/') > 0) {
                            jid = jid.substring(0, jid.indexOf('/'));
                        }

                        User user = findUser(jid);
                        if (user == null) {
                            LOG.error("Rejected subscription from " + jid);
                            presPacket = new Presence(Presence.Type.unsubscribed);
                        } else {
                            LOG.info("Accepted subscription from " + jid);
                            presPacket = new Presence(Presence.Type.subscribed);
                        }
                        presPacket.setTo(presence.getFrom());
                        presPacket.setFrom(presence.getTo());

                        m_con.sendPacket(presPacket);

                        try {
                            sleep(1000);
                        } catch (InterruptedException e) {
                        }

                        if ((user != null) && (m_ChatsMap.get(jid) == null)) {
                            // now subscribe the sender's presence
                            presPacket.setType(Presence.Type.subscribe);

                            m_con.sendPacket(presPacket);

                            IMUser imuser = new IMUser(user, jid, null, m_con, m_localizer);
                            m_ChatsMap.put(jid, imuser);
                        }

                        break;
                    case unsubscribe:
                        LOG.error("Received unexpected unsubscribe for " + presence.getFrom());
                        break;
                    }
                }
            }

            PacketFilter filter = new PacketTypeFilter(Presence.class);
            m_con.addPacketListener(new PresenceListener(), filter);

            // create map with initial presence and status info

            User user;
            Collection<RosterEntry> entries = m_roster.getEntries();
            for (RosterEntry entry : entries) {
                user = findUser(entry.getUser());
                if (user != null) {
                    IMUser imuser = new IMUser(user, entry.getUser(), m_roster.getPresence(entry.getUser()), m_con,
                        m_localizer);
                    m_ChatsMap.put(entry.getUser(), imuser);
                } else {
                    try {
                        m_roster.removeEntry(entry);
                        LOG.error("Removing old roster entry " + entry.getUser());
                    } catch (XMPPException e) {
                        LOG.error("Could not remove roster entry " + entry.getUser());
                    }
                }
            }

            m_roster.addRosterListener(new RosterListener() {

                @Override
                public void entriesAdded(Collection<String> entries) {
                    for (String address : entries) {
                        if (m_ChatsMap.get(address) != null) {
                            // already in chat map, this will happen
                            // if subscription was successful
                            continue;
                        }

                        User user = findUser(address);
                        if (user == null) {
                            LOG.error("Rejected addition from " + address);
                        } else {
                            IMUser imuser = new IMUser(user, address, null, m_con, m_localizer);
                            m_ChatsMap.put(address, imuser);
                            LOG.debug("Entry added: " + address);
                        }
                    }
                }

                @Override
                public void entriesDeleted(Collection<String> addresses) {
                    // Contacts have been removed from the roster
                    for (String address : addresses) {
                        for (RosterEntry entry : m_roster.getEntries()) {
                            if (address.equals(entry.getUser())) {
                                LOG.debug("Removing from roster: " + entry.getUser());
                                m_ChatsMap.remove(entry.getUser());
                            }
                        }
                    }
                }

                @Override
                public void entriesUpdated(Collection<String> arg0) {
                }

                @Override
                public void presenceChanged(Presence presence) {

                    String from = presence.getFrom();
                    if (from.indexOf('/') > 0) {
                        from = from.substring(0, from.indexOf('/'));
                    }
                    m_ChatsMap.get(from).setPresence(presence);
                }
            });

            while (running) {
                try {
                    sleep(5000);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    running = false;
                }
            }
            m_con.disconnect();
        }
    }

    static public synchronized User findUser(String jid) {
        return (FullUsers.INSTANCE.findByjid(jid));
    }

    static private String getjid(User user) {
        String jid = user.getJid();
        if (jid == null) {
            jid = user.getAltJid();
        }
        return jid;
    }

    static public String getUserStatus(User user) {
        // return status corresponding to primary IM Id. If not filled in
        // try altId and if not filled in either then assume AVAILABLE
        String jid = getjid(user);
        if (jid == null) {
            return null;
        }

        Presence pres = m_roster.getPresence(jid);

        if (pres != null) {
            return pres.getStatus();
        }

        return null;
    }

    static public UserPresence getUserPresence(User user) {
        // return presence corresponding to primary IM Id. If not filled in
        // try altId and if not filled in either then assume AVAILABLE

        if (ConfBasicThread.inConferenceSince(user.getUserName()) != null) {
            return UserPresence.INCONFERENCE;
        }

        String jid = getjid(user);
        if (jid == null) {
            return UserPresence.AVAILABLE;
        }
        Presence pres = m_roster.getPresence(jid);

        if (pres == null) {
            return UserPresence.UNKNOWN;
        }

        if (pres.getType() == Presence.Type.unavailable) {
            return UserPresence.UNKNOWN;
        }

        Presence.Mode mode = pres.getMode();
        if (mode == null) {
            return UserPresence.AVAILABLE;
        }

        if (mode == Presence.Mode.away) {
            return UserPresence.AWAY;
        }

        if (mode == Presence.Mode.chat) {
            return UserPresence.AVAILABLE;
        }

        if (mode == Presence.Mode.dnd) {
            return UserPresence.BUSY;
        }

        if (mode == Presence.Mode.xa) {
            return UserPresence.AWAY;
        }

        return UserPresence.UNKNOWN;

    }

    public static void sendIM(User user, String msg) {
        if (user != null) {
            IMUser toIMUser;

            String jid = user.getJid();
            if (jid != null) {
                toIMUser = m_ChatsMap.get(jid);
                if (toIMUser != null) {
                    toIMUser.sendIM(msg);
                }
            }

            jid = user.getAltJid();
            if (jid != null) {
                toIMUser = m_ChatsMap.get(jid);
                if (toIMUser != null) {
                    toIMUser.sendIM(msg);
                }
            }
        }
    }

    static public IMUser getIMUser(User user) {
        String jid = getjid(user);
        if (jid == null) {
            return null;
        }
        return m_ChatsMap.get(jid);
    }

    static public void SendReturnCallIM(User toUser, User fromUser, String callingName, String callingNumber) {

        IMUser toIMuser = getIMUser(toUser);
        IMUser fromIMuser = getIMUser(fromUser);

        if (toIMuser != null && fromIMuser != null) {
            toIMuser.setCallingIMUser(fromIMuser);

            toIMuser.sendIM(callingName + " (" + callingNumber + ") called and would like you to call back");
        }
    }

    public static void AddToRoster(User user) {
        IMClientThread.AddToRoster(user);
    }

    static public void init() {
        DNSUtil.setDNSResolver(JavaxResolver.getInstance());
        IMClientThread imThread = new IMClientThread();
        imThread.start();
    }
}
