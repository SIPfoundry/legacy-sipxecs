/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.methods.PutMethod;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.api.FreeswitchApi;
import org.sipfoundry.sipxconfig.freeswitch.api.FreeswitchApiConnectException;
import org.sipfoundry.sipxconfig.freeswitch.api.FreeswitchApiResultParser;
import org.sipfoundry.sipxconfig.freeswitch.api.FreeswitchApiResultParserImpl;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.recording.RecordingManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public class ActiveConferenceContextImpl implements ActiveConferenceContext, BeanFactoryAware {

    private static final String COMMAND_LIST_DELIM = ">,<";
    private static final String COMMAND_LIST = "list delim " + COMMAND_LIST_DELIM;
    private static final Log LOG = LogFactory.getLog(ActiveConferenceContextImpl.class);
    private static final String ERROR_INVITE_PARTICIPANT = "&error.inviteParticipant";

    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private final FreeswitchApiResultParser m_freeswitchApiParser = new FreeswitchApiResultParserImpl();
    private DomainManager m_domainManager;
    private AddressManager m_addressManager;
    private SipService m_sipService;
    private CoreContext m_coreContext;
    private BeanFactory m_beanFactory;

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setFreeswitchApiProvider(ApiProvider<FreeswitchApi> freeswitchApiProvider) {
        m_freeswitchApiProvider = freeswitchApiProvider;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Required
    public void setSipService(SipService sipService) {
        m_sipService = sipService;
    }


    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public int getActiveConferenceCount(Bridge bridge) {
        LOG.debug("Requesting count of active conferences from bridge " + bridge.getName());
        String result = null;
        try {
            result = api(bridge).conference(COMMAND_LIST);
        } catch (XmlRpcRemoteException xrre) {
            throw new FreeswitchApiConnectException(bridge, xrre);
        }

        int conferenceCount = m_freeswitchApiParser.getActiveConferenceCount(result);
        LOG.debug(String.format("Bridge \"%s\" reports %d active conferences.", bridge.getName(),
                conferenceCount));
        return conferenceCount;
    }

    private FreeswitchApi api(Bridge bridge) {
        String url = m_addressManager.getSingleAddress(FreeswitchFeature.XMLRPC_ADDRESS, bridge.getLocation())
                .toString();
        return m_freeswitchApiProvider.getApi(url);
    }

    @Override
    public List<ActiveConference> getActiveConferences(Bridge bridge) {
        LOG.debug("Requesting list of active conferences from bridge " + bridge.getName());
        String result = null;
        try {
            result = api(bridge).conference(COMMAND_LIST);
        } catch (XmlRpcRemoteException xrre) {
            throw new FreeswitchApiConnectException(bridge, xrre);
        }
        List<ActiveConference> conferences = m_freeswitchApiParser.getActiveConferences(result);

        for (ActiveConference activeConference : conferences) {
            Conference conference = m_conferenceBridgeContext
                    .findConferenceByName(activeConference.getName());
            if (conference != null) {
                activeConference.setConference(conference);
            }
        }

        LOG.debug(String.format("Bridge \"%s\" reports the following active conferences: %s",
                bridge.getName(), conferences));
        return conferences;
    }

    @Override
    public Map<Conference, ActiveConference> getActiveConferencesMap(Bridge bridge) {
        Map<Conference, ActiveConference> activeConferencesMap = new HashMap<Conference, ActiveConference>();

        List<ActiveConference> activeConferences = getActiveConferences(bridge);
        for (ActiveConference activeConference : activeConferences) {
            activeConferencesMap.put(activeConference.getConference(), activeConference);
        }

        return activeConferencesMap;
    }

    @Override
    public List<ActiveConferenceMember> getConferenceMembers(Conference conference) {
        LOG.debug("Requesting list of members for conference \"" + conference.getName() + "\"");
        Bridge bridge = conference.getBridge();
        String conferenceName = conference.getName();
        String result = null;
        List<ActiveConferenceMember> members = new ArrayList<ActiveConferenceMember>();

        try {
            result = api(bridge).conference(conferenceName + " " + COMMAND_LIST);
            // if (m_freeswitchApiParser.verifyConferenceAction(result, conference)) {
            members = m_freeswitchApiParser.getConferenceMembers(result, conference);
            // }
        } catch (XmlRpcRemoteException xrre) {
            throw new FreeswitchApiConnectException(bridge, xrre);
        }

        LOG.debug(String.format(
                "Bridge \"%s\" reports the following members for conference \"%s\": %s", bridge
                        .getName(), conference.getName(), members));
        return members;
    }

    @Override
    public String executeCommand(Conference conference, String[] arguments) {
        HttpClient client = new HttpClient();
        String uri = getConferenceManagerRestUrl(conference, arguments);
        PutMethod putMethod = new PutMethod(uri);
        int statusCode = HttpStatus.SC_OK;
        String response = null;
        try {
            statusCode = client.executeMethod(putMethod);
            if (statusCode == HttpStatus.SC_OK) {
                response = putMethod.getResponseBodyAsString();
            }
        } catch (Exception ex) {
            response = "ERROR: " + ex.getMessage();
        }
        return response;
    }

    private String getConferenceManagerRestUrl(Conference conf, String[] arguments) {
        if (StringUtils.equals(arguments[0], "record")) {
            User owner = conf.getOwner();
            String domainName = m_domainManager.getDomain().getName();
            return String.format("http://%s:%d/recordconference?action=%s&conf=%s&ownerName=%s&ownerId=%s"
                    + "&bridgeContact=%s&mboxServer=%s",
                conf.getBridge().getName(), getConfCommandsPort(), arguments.length == 1 ? "start" : arguments[1],
                conf.getName(), owner.getUserName(), owner.getAddrSpec(domainName), conf.getUri(), findMailboxServer());
        } else {
            StringBuilder builder = new StringBuilder();
            for (String argument : arguments) {
                builder.append("/")
                       .append(argument);
            }
            return String.format("http://%s:%d/conference/%s%s",
                    conf.getBridge().getName(), getConfCommandsPort(), conf.getName(), builder.toString());
        }
    }

    private String findMailboxServer() {
        Address api = m_addressManager.getSingleAddress(Ivr.REST_API);
        if (api == null) {
            return StringUtils.EMPTY;
        }
        return api.getAddress() + ':' + api.getPort();
    }

    @Override
    public boolean lockConference(Conference conference) {
        return conferenceAction(conference, "lock");
    }

    @Override
    public boolean unlockConference(Conference conference) {
        return conferenceAction(conference, "unlock");
    }

    @Override
    public boolean deafUser(Conference conference, ActiveConferenceMember member) {
        return memberAction(conference, member, "deaf");
    }

    @Override
    public boolean muteUser(Conference conference, ActiveConferenceMember member) {
        return memberAction(conference, member, "mute");
    }

    @Override
    public boolean undeafUser(Conference conference, ActiveConferenceMember member) {
        return memberAction(conference, member, "undeaf");
    }

    @Override
    public boolean unmuteUser(Conference conference, ActiveConferenceMember member) {
        return memberAction(conference, member, "unmute");
    }

    @Override
    public boolean kickUser(Conference conference, ActiveConferenceMember member) {
        return memberAction(conference, member, "kick");
    }

    @Override
    public void inviteParticipant(User user, Conference conference, String inviteNumber) {
        String domain = m_domainManager.getDomain().getName();
        String sourceAddressSpec = SipUri.fix(inviteNumber, domain);

        if (conference.hasOwner()) {

            String partPin = conference.getParticipantAccessCode();
            String dest = m_conferenceBridgeContext.getAddressSpec(conference);

            if (partPin != null && partPin.length() > 0) {
                dest += "?X-ConfPin=" + partPin;
            }

            m_sipService.sendRefer(conference.getOwner(), // Who are we (credentials)
                  sourceAddressSpec,                      // Who we are inviting
                  conference.getName(),                   // From this name
                                                          // From this address
                  dest,
                  // Use the above, not this one: conference.getUri(), as the
                  // former will be extension@proxy, the latter will be
                  // confname@domain:freeswitchport.  We want the former
                  // so calls back to it will go via the proxy, not directly
                  // to the conference which may not be externally addressable
                  dest, true);
        } else {
            LOG.warn("conference does not have owner -- cannot INVITE participant");
            throw new UserException(ERROR_INVITE_PARTICIPANT);
        }
    }

    public void inviteImParticipant(User user, Conference conference, String imIdToInvite) {
        User userToInvite = m_coreContext.loadUserByConfiguredImId(imIdToInvite);
        if (userToInvite != null) {
            inviteParticipant(user, conference, userToInvite.getUserName());
        } else {
            LOG.warn("There is no user for the specified ImId -- cannot INVITE participant");
            throw new UserException(ERROR_INVITE_PARTICIPANT);
        }
    }

    private boolean conferenceAction(Conference conference, String... commands) {
        String command = StringUtils.join(commands, ' ');

        LOG.debug(String.format("Executing command \"%s\" on conference \"%s\"", command,
                conference.getName()));
        Bridge bridge = conference.getBridge();
        String conferenceName = conference.getName();
        try {
            String result = api(bridge).conference(String.format("%s %s", conferenceName, command));
            return m_freeswitchApiParser.verifyConferenceAction(result, conference);
        } catch (XmlRpcRemoteException xrre) {
            throw new FreeswitchApiConnectException(bridge, xrre);
        }
    }

    private boolean memberAction(Conference conference, ActiveConferenceMember member,
            String command) {
        LOG.debug(String.format("Executing command '%s' (conference='%s', member='%s')...",
                command, conference.getName(), member.getName()));
        Bridge bridge = conference.getBridge();
        String conferenceName = conference.getName();
        String result = "";
        try {
            result = api(bridge).conference(String.format("%s %s %d", conferenceName, command, member
                    .getId()));
            return m_freeswitchApiParser.verifyMemberAction(result, member);
        } catch (XmlRpcRemoteException xrre) {
            throw new FreeswitchApiConnectException(bridge, xrre);
        } finally {
            LOG.debug(String.format(
                    "Command '%s' (conference='%s', member='%s') completed with result: '%s'",
                    command, conference.getName(), member.getName(), result));
            // LOG.debug("Pausing for 1.5 seconds to let FreeSWITCH catch up...");
            // try { Thread.sleep(1000); }
            // catch (InterruptedException ie) { LOG.debug("Interrupted"); }
        }
    }

    @Override
    public boolean isConferenceLocked(Conference conference) {
        ActiveConference activeConference = null;
        try {
            activeConference = getActiveConference(conference);
        } catch (FreeswitchApiConnectException face) {
            LOG.warn("Couldn't connect to FreeSWITCH to get conference locked status", face);
        }

        return (activeConference != null) ? activeConference.isLocked() : false;
    }

    @Override
    public ActiveConference getActiveConference(Conference conference) {
        String conferenceName = conference.getName();
        ActiveConference activeConference = null;
        Bridge bridge = conference.getBridge();
        List<ActiveConference> activeConferences = getActiveConferences(bridge);
        for (ActiveConference c : activeConferences) {
            if (c.getName().equals(conferenceName)) {
                activeConference = c;
                break;
            }
        }

        return activeConference;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    @Required
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    private int getConfCommandsPort() {
        RecordingManager manager = (RecordingManager) m_beanFactory.getBean(RecordingManager.BEAN_NAME);
        return manager.getJettyPort();
    }
}
