package org.sipfoundry.openfire.config;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;
import org.jivesoftware.openfire.group.Group;
import org.jivesoftware.openfire.group.GroupNotFoundException;
import org.jivesoftware.util.Log;
import org.sipfoundry.openfire.client.OpenfireClientException;
import org.sipfoundry.openfire.client.OpenfireXmlRpcKrakenClient;
import org.sipfoundry.openfire.client.OpenfireXmlRpcPresenceClient;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePlugin;
import org.xmpp.packet.JID;

public class XmppAccountInfo {

    private static final Logger logger = Logger.getLogger(XmppAccountInfo.class);
    private static SipXOpenfirePlugin plugin = SipXOpenfirePlugin.getInstance();

    private Map<String, XmppUserAccount> userAccounts = new HashMap<String, XmppUserAccount>();

    private Map<String, XmppGroup> groups = new HashMap<String, XmppGroup>();

    private Map<String, XmppChatRoom> chatRooms = new HashMap<String, XmppChatRoom>();

    protected static String appendDomain(String userName) {
        if (userName.indexOf("@") == -1) {
            // No @ in the domain so assume this is our domain.
            return userName + "@" + SipXOpenfirePlugin.getInstance().getXmppDomain();
        } else {
            return userName;
        }
    }

    public XmppAccountInfo() {

    }

    public void addAccount(XmppUserAccount xmppUserAccount) throws Exception {
        logger.debug("createUserAccount " + xmppUserAccount.getUserName() + " password = "
                + xmppUserAccount.getPassword());
        plugin.createUserAccount(xmppUserAccount.getUserName(), xmppUserAccount.getPassword(),
                xmppUserAccount.getDisplayName(), xmppUserAccount.getEmail(),
                xmppUserAccount.getAdvertiseOnCallPreference(), xmppUserAccount.getShowOnCallDetailsPreference());
        String jid = appendDomain(xmppUserAccount.getUserName());
        String sipUserName = xmppUserAccount.getSipUserName();
        logger.debug("setSipId " + jid + " sipUserName " + sipUserName);
        plugin.setSipId(jid, sipUserName);
        plugin.setOnThePhoneMessage(sipUserName, xmppUserAccount.getOnThePhoneMessage());
        this.userAccounts.put(xmppUserAccount.getUserName(), xmppUserAccount);
        // Make sure that user can create multi-user chatrooms
        plugin.setAllowedUserForChatServices(jid);
    }

    public OpenfireXmlRpcKrakenClient getKrakenClient() {

		WatcherConfig config = SipXOpenfirePlugin.getInstance().getSipXopenfireConfig();

		if (config == null) {
			logger.error("Cannot get Kraken XML-RPC client because sipxopenfire.xml has not been parsed");
			return null;
		}

		String domain = SipXOpenfirePlugin.getInstance().getXmppDomain();
		int port = config.getOpenfireXmlRpcPort();
		OpenfireXmlRpcKrakenClient krakenClient;

		try {
			logger.debug("getKrakenClient with domain: " + domain + " port:" + port);

			krakenClient = new OpenfireXmlRpcKrakenClient(domain, port);
		} catch (Exception ex) {
			logger.error("Cannot create Kraken XML-RPC client:" + ex.getMessage());
			return null;
		}

		return krakenClient;
	}

	public void syncTransportRegistrations() {

		OpenfireXmlRpcKrakenClient krakenClient = getKrakenClient();

		if (krakenClient == null) {
			logger.error("Abort transport registration synchronization with Openfire.");
			return;
		}

		// Get the current list of active transports in Openfire.
		// At the same time we need to keep a parallel list of transports that
		// will be updated by the registrations provided in sipXconfig.
		// We will use those two lists to de-activate the transports that have
		// no registrations.

		List<String> activeTransports;
		Collection<String> updatedTransports;

		try {
			activeTransports = krakenClient.getActiveTransports();

			updatedTransports = new HashSet<String>();

			// Iterate over each user account and get the list of configured
			// transport registrations.
			for (String accountKey : userAccounts.keySet()) {

				XmppUserAccount accountEntry = userAccounts.get(accountKey);
				Map<String, XmppTransportRegistration> transReg = accountEntry
						.getTransportRegistrations();

				// Make a copy of the sipXconfig of transport registrations.
				// Use this collection to figure out the registrations that
				// need to be added in Openfire.
				Collection<XmppTransportRegistration> registrationsToAdd = new HashSet<XmppTransportRegistration>(
						transReg.values());

				// Get the actual collection of transport registrations in
				// Openfire for this user. We will use this list to figure
				// out the registrations entries that need to be removed.
				Collection<XmppTransportRegistration> registrationsToRemove;
				registrationsToRemove = krakenClient.getRegistrations(accountKey);

				registrationsToAdd.removeAll(registrationsToRemove);
				registrationsToRemove.removeAll(transReg.values());

				// Keep the transports active for the registrations that 
				// either do not change or that need to be added.
				for (XmppTransportRegistration reg : transReg.values()) {
					
					updatedTransports.add(reg.getTransportType());
				}

				logger.debug("Remove transport registrations for user: " + accountKey);
				for (XmppTransportRegistration reg : registrationsToRemove) {
					
					// Before we can delete a registration we have to make
					// sure that the transport is active.
					if (!activeTransports.contains(reg.getTransportType())) {

						try {
							if (!krakenClient.toggleTransport(reg.getTransportType())) {
								logger.error("Registration deletion: failed to activate transport: "
												+ reg.getTransportType());
								continue;
							}
						} catch (OpenfireClientException ex) {
							logger.error("Registration deletion: failed to activate transport: "
									+ reg.getTransportType() + " reason: " + ex.getMessage());
							continue;
						}

						activeTransports.add(reg.getTransportType());
					}
					if (!krakenClient.deleteRegistration(reg.getUser(), reg.getTransportType())) {
						logger.error("Registration deletion: failed to delete user: "
								+ reg.getUser() + " transport: " + reg.getTransportType());
						continue;
					}
				}

				logger.debug("Add transport registrations for user: " + accountKey);
				for (XmppTransportRegistration reg : registrationsToAdd) {
					
					// Before we can add a registration we have to make
					// sure that the transport is active.
					if (!activeTransports.contains(reg.getTransportType())) {
						try {
							if (!krakenClient.toggleTransport(reg.getTransportType())) {
								logger.error("Registration addition: failed to activate transport: "
												+ reg.getTransportType());
								continue;
							}
						} catch (OpenfireClientException ex) {
							logger.error("Registration addition: failed to activate transport: "
									+ reg.getTransportType() + " reason: " + ex.getMessage());
							continue;
						}

						activeTransports.add(reg.getTransportType());
					}
					if (!krakenClient.addRegistration(reg.getUser(), reg.getTransportType(), reg
							.getLegacyUsername(), reg.getLegacyPassword(), reg.getLegacyNickname())) {
						logger.error("Registration addition: failed to add user: " + reg.getUser()
								+ " transport: " + reg.getTransportType());
						continue;
					}
				}
			}
			
			// We are done with all transport registrations so it is
			// time to de-activate all unused transports.
			activeTransports.removeAll(updatedTransports);
			logger.debug("Cleanup: unused transports");

			/* Important note: Because of a problem with enabling and
			 * disabling the "yahoo", "irc" or "myspaceim" transports types
			 * we will not disable any transport type once it is enabled.
			 * This workaround should be investigated again once we get
			 * a newer version of the Kraken plugin (> 1.1.2).
			 * The problem in question is that logging to file sipxopenfire.log
			 * turns itself off for an unknown reason. But if we keep those
			 * 3 transport types always active, then this problem does 
			 * not occur. The 3 transport types will be enabled at 
			 * Openfire database creation time.
			for (String transport : activeTransports) {
				
				try {
					if (krakenClient.toggleTransport(transport)) {
						logger.error("Cleanup: de-activation of transport for " + transport);
						continue;
					}
				} catch (OpenfireClientException ex) {
					logger.error("Cleanup: de-activation of transport for " + transport
							+ " reason: " + ex.getMessage());
					continue;
				}
			}
            */
			
		} catch (Exception ex) {
			logger.error(ex);
			return;
		}
	}

    public void addGroup(XmppGroup group) throws Exception {
        logger.info("createGroup " + group.getGroupName() + " description "
                + group.getDescription());
        if (group.getMembers().isEmpty()) {
            logger.info("no users defined -- not adding group ");
            return;
        }
        boolean isAllAdminGroup = false;
        String adminJid = null;
        JID adminJID = null;
        if (group.getAdministrator() == null) {
            isAllAdminGroup = true;
        } else {
            adminJid = appendDomain(group.getAdministrator());
            adminJID = new JID(adminJid);

        }

        // check if group already exists in openfire
        try{
            Group openfireGroup = plugin.getGroupByName(group.getGroupName());

            // group already exists, make sure that it contains the correct set of members.
            // This is achieved in two operations:
            //  1- Add all members that are currently not in the group but are found in the group from configuration file
            //  2- Remove all members that are currently in the group but not found in the group from configuration file
            logger.info("XmppAccountInfo::addGroup: " + group.getGroupName() + " already exists - enforce members list");
            Collection<String> currentGroupMembers = new HashSet<String>();
            for( JID jid : openfireGroup.getMembers() ){
                currentGroupMembers.add(jid.toBareJID());
            }
            
            Collection<String> desiredGroupMembers = new HashSet<String>();
            Collection<String> desiredGroupMembersBackup = new HashSet<String>();
            for( XmppGroupMember member :group.getMembers() ){
                desiredGroupMembers.add(member.getJid());
                desiredGroupMembersBackup.add(member.getJid());
            }

            //  1- Add all members that are currently not in the group but are found in the group from configuration file
            desiredGroupMembers.removeAll(currentGroupMembers);
            logger.info("Need to add the following members to group '" + group.getGroupName() + "': " + desiredGroupMembers);
            for( String jid : desiredGroupMembers){
                SipXOpenfirePlugin.getInstance().addUserToGroup(jid, group.getGroupName(), isAllAdminGroup);
            }

            //  2- Remove all members that are currently in the group but not found in the group from configuration file
            currentGroupMembers.removeAll(desiredGroupMembersBackup);
            logger.info("Need to remove the following members to group '" + group.getGroupName() + "': " + currentGroupMembers);
            for( String jid : currentGroupMembers){
                SipXOpenfirePlugin.getInstance().removeUserFromGroup(jid, group.getGroupName());
            }
        }
        catch( GroupNotFoundException ex ){
            logger.info("XmppAccountInfo::addGroup: " + group.getGroupName() + " does not exist - create it");
            plugin.createGroup(group.getGroupName(), adminJID, group.getDescription());

            for (XmppGroupMember member : group.getMembers()) {
                String userJid = appendDomain(member.getJid());
                SipXOpenfirePlugin.getInstance().addUserToGroup(userJid, group.getGroupName(), isAllAdminGroup);
            }
        }
        this.groups.put(group.getGroupName(), group);       
    }

    public void addChatRoom(XmppChatRoom xmppChatRoom) throws Exception {
        boolean allowInvite = false;
        logger.info(String.format("createChatRoom %s\n %s\n %s\n %s",
                xmppChatRoom.getSubdomain(), xmppChatRoom.getRoomName(), xmppChatRoom
                        .getDescription(), xmppChatRoom.getConferenceExtension()));
        if ( xmppChatRoom.getSubdomain() == null || xmppChatRoom.getRoomName() == null ) {
            logger.error("Null parameters specified.");
            return;
        }
        plugin.createChatRoom(xmppChatRoom.getSubdomain(), xmppChatRoom.getOwner(), xmppChatRoom
                .getRoomName(), xmppChatRoom.isModerated(), xmppChatRoom
                .isMembersOnly(), allowInvite, xmppChatRoom.isPublicRoom(), xmppChatRoom
                .isLogRoomConversations(), xmppChatRoom.isPersistent(), xmppChatRoom
                .getPassword(), xmppChatRoom.getDescription(), xmppChatRoom
                .getConferenceExtension(), xmppChatRoom.getConferenceName(),
                xmppChatRoom.getConferenceReachabilityInfo());
        this.chatRooms.put(xmppChatRoom.getSubdomain() + ":" + xmppChatRoom.getRoomName(),
                xmppChatRoom);

    }

    public XmppUserAccount getXmppUserAccount(String name) {
        return this.userAccounts.get(name);
    }

    public XmppGroup getXmppGroup(String name) {
        return this.groups.get(name);
    }

    public XmppChatRoom getXmppChatRoom(String subdomain, String name) {
        String key = subdomain + ":" + name;
        return this.chatRooms.get(key);
    }

    public Collection<XmppChatRoom> getXmppChatRooms() {
        return this.chatRooms.values();
    }

}
