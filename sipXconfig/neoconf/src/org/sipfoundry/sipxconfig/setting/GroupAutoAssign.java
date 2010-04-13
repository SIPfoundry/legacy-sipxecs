/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class GroupAutoAssign {

    private static final Log LOG = LogFactory.getLog(GroupAutoAssign.class);
    private static final String CONFERENCE_PREFIX = "conference/prefix";
    private static final String EXTCONTACT_PREFIX = "extcontact/prefix";

    private ConferenceBridgeContext m_bridgeContext;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private MailboxManager m_mailboxManager;

    public GroupAutoAssign(ConferenceBridgeContext bridgeContext,
                           CoreContext coreContext,
                           ForwardingContext forwardingContext,
                           MailboxManager mailboxManager) {
        m_bridgeContext = bridgeContext;
        m_coreContext = coreContext;
        m_forwardingContext = forwardingContext;
        m_mailboxManager = mailboxManager;
    }

    /**
     * Examines the groups a user belongs to and determines which, if any, group's
     * creation settings to use.
     *
     * @param user The user being created.
     * @return A Group whose settings should be used, or null if none of the
     *         user's groups have conferences enabled.
     */
    private Group getHighestGroup(User user, String settingStr) {
        // Find the highest weight group that has settings enabled, if any.
        List<Group> userGroups = new ArrayList<Group>(user.getGroupsAsList()); // cloning it
        // because we may
        // remove items
        Group settingGroup = null;
        while (!userGroups.isEmpty() && settingGroup == null) {
            Group highestGroup = Group.selectGroupWithHighestWeight(userGroups);
            SettingValue groupValue = highestGroup.getSettingValue(new SettingImpl(
                    (settingStr + Setting.PATH_DELIM) + "enabled"));
            if (groupValue != null && Boolean.valueOf(groupValue.getValue())) {
                settingGroup = highestGroup;
            } else {
                userGroups.remove(highestGroup);
            }
        }

        if (settingGroup != null) {
            LOG.debug("Using group \"" + settingGroup.getName() + "\" for " + settingStr + " settings for new user: "
                    + user.getUserName());
        }

        return settingGroup;
    }

    /**
     * Creates the contact string from the prefix plus the user
     * .
     * @param user The new user being created.
     * @param contactPrefix The contact prefix.
     * @return The resulting contact string.
     */
    private String contactString(User user, SettingValue contactPrefix) {
        String extension = user.getExtension(true);
        String extensionOrName = extension == null ? user.getUserName() : extension;
        String contactStr = contactPrefix + extensionOrName;

        return contactStr;
    }

    /**
     * Configures the External Contact MWI settings.
     *
     * @param user The user that has just been created.
     * @param extcontactGroup The group containing external contact settings to use.
     */
    private void createUserExternalMwi(User user, Group extcontactGroup) {
        SettingValue extContactPrefix = extcontactGroup.getSettingValue(new SettingImpl(EXTCONTACT_PREFIX));
        String extContact = contactString(user, extContactPrefix);

        LOG.debug(String.format("Creating external contact mwi \"%s\", extension %s, for user %s",
                  user.getUserName(), extContact, user.getDisplayName()));

        MailboxPreferences mailboxPreferences = new MailboxPreferences(user);
        mailboxPreferences.setExternalMwi(extContact);
        mailboxPreferences.updateUser(user);
    }

    /**
     * Configures the External Contact Call Forward settings.
     *
     * @param user The user that has just been created.
     * @param extcontactGroup The group containing external contact settings to use.
     */
    private void createUserCallForward(User user, Group extcontactGroup) {
        SettingValue extContactPrefix = extcontactGroup.getSettingValue(new SettingImpl(EXTCONTACT_PREFIX));
        String extContact = contactString(user, extContactPrefix);

        LOG.debug(String.format("Creating external contact fork \"%s\", extension %s, for user %s, id %s",
                  user.getUserName(), extContact, user.getDisplayName(), user.getId()));

        User reloadUser = m_coreContext.loadUser(user.getId());
        CallSequence callSequence = m_forwardingContext.getCallSequenceForUser(reloadUser);

        Ring ring = callSequence.insertRing();
        ring.setNumber(extContact);
        ring.setExpiration(callSequence.getCfwdTime());
        ring.setType(AbstractRing.Type.IMMEDIATE);

        m_forwardingContext.saveCallSequence(callSequence);
        m_forwardingContext.flush();
    }

    /**
     * Creates a new conference for a new user.
     *
     * @param user The user that has just been created.
     * @param conferenceGroup The group containing conference settings to use.
     */
    private void createUserConference(User user, Group conferenceGroup) {
        SettingValue bridgeIdValue = conferenceGroup.getSettingValue(new SettingImpl("conference/bridgeId"));
        Integer bridgeId = Integer.parseInt(bridgeIdValue.getValue());

        Bridge bridge = null;
        try {
            bridge = m_bridgeContext.loadBridge(bridgeId);
        } catch (HibernateObjectRetrievalFailureException horfe) {
            LOG.warn(String.format("Unable to create a conference for new user %s; the user group \"%s\" "
                    + "references a non-existent conference bridge ID: %d", user.getUserName(), conferenceGroup
                    .getName(), bridgeId));
        }

        if (bridge != null) {
            SettingValue conferencePrefix = conferenceGroup.getSettingValue(new SettingImpl(CONFERENCE_PREFIX));
            String conferenceExtension = contactString(user, conferencePrefix);

            Conference userConference = m_bridgeContext.newConference();
            userConference.setExtension(conferenceExtension.toString());
            userConference.setName(user.getUserName() + "-conference");
            userConference.setOwner(user);
            userConference.setEnabled(true);
            userConference.setDescription("Automatically created conference for " + user.getDisplayName());

            LOG.debug(String.format("Creating conference \"%s\", extension %s, for user %s", userConference
                    .getName(), userConference.getExtension(), user.getDisplayName()));

            m_bridgeContext.validate(userConference);
            bridge.addConference(userConference);
            m_bridgeContext.store(bridge);
        }
    }

    /**
     * Execute the auto-assignments for the user
     *
     * @param user The user that has just been created.
     */
    public void assignUserData(User user) {
        Group extcontactGroup = getHighestGroup(user, "extcontact");
        if (extcontactGroup != null) {
            createUserExternalMwi(user, extcontactGroup);
        }

        m_coreContext.saveUser(user);

        if (extcontactGroup != null) {
            createUserCallForward(user, extcontactGroup);
        }

        // Initialize the new user's mailbox
        if (m_mailboxManager.isEnabled()) {
            m_mailboxManager.deleteMailbox(user.getUserName());
        }

        // If necessary, create a conference for this user.
        Group conferenceGroup = getHighestGroup(user, "conference");
        if (conferenceGroup != null) {
            createUserConference(user, conferenceGroup);
        }
    }

}
