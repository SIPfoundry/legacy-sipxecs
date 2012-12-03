/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.freeswitch;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

public class ConfBasicThread extends Thread {
    // Default freeswitch socket client strings, should be read from:
    // /usr/local/freeswitch/conf/autoload_configs/event_socket.conf.xml
    static String fsListenPort = "8021"; // parameter "listen-port"
    static String fsPassword = "ClueCon"; // parameter "password"

    // a thread that listens on a freeswitch socket for conference related events
    // based on events received, maintains relavant data about conferences and
    // participants

    // key is conference name, value is conference object
    private static Map<String, ConferenceTask> m_ConferenceMap = Collections
            .synchronizedMap(new HashMap<String, ConferenceTask>());

    // key is user name (eg 200), value is conference start time
    private static Map<String, Date> m_UserMap = Collections
    .synchronizedMap(new HashMap<String, Date>());

    private static FreeSwitchConfigurationInterface m_fsConfig;
    private static FreeSwitchEventSocket m_fsCmdSocket;
    private static FreeSwitchEventSocket m_fsListenSocket;

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");

    public void setConfConfiguration(FreeSwitchConfigurationInterface fsConfig) {
        m_fsConfig = fsConfig;
    }

    public void ProcessConfStart(FreeSwitchEvent event, ConferenceTask conf) {
        LOG.debug("ConfBasicThread::processConfStart()");
    }

    public void ProcessConfEnd(FreeSwitchEvent event, ConferenceTask conf) {
        LOG.debug("ConfBasicThread::processConfEnd()");
    }

    public void ProcessConfUserAdd(ConferenceTask conf, ConferenceMember member) {
        LOG.debug("ConfBasicThread::processConfUserAdd()");
    }

    public void ProcessConfUserDel(ConferenceTask conf, ConferenceMember member) {
        LOG.debug("ConfBasicThread::processConfUserDel()");
    }

    public static FreeSwitchEventSocket getCmdSocket() {
        return m_fsCmdSocket;
    }

    private boolean processEvent(FreeSwitchEvent event) {
        LOG.debug("ConfBasicThread::processEvent()");
        String confName = event.getEventValue("conference-name");
        String memberName = event.getEventValue("caller-caller-id-name");
        String memberNumber = event.getEventValue("caller-caller-id-number");
        String action = event.getEventValue("action");
        String confSize = event.getEventValue("conference-size");
        String memberId =  event.getEventValue("member-id");

        if(action != null) {

            ConferenceTask conf = m_ConferenceMap.get(confName);

            if(action.equalsIgnoreCase("add-member")) {

                if(confSize.equals("1")) {
                    // first to join the conference
                    conf = new ConferenceTask();
                    m_ConferenceMap.put(confName, conf);
                    ProcessConfStart(event, conf);
                }

                // add member to conference object
                ConferenceMember member = new ConferenceMember();
                member.m_memberId = memberId;
                member.m_muted = false;
                member.m_memberNumber = memberNumber;
                member.m_memberName = memberName;
                member.m_memberIndex = conf.getNextParticipantIndex();
                conf.add(member.m_memberId, member);
                ProcessConfUserAdd(conf, member);
                User user = UnfortunateLackOfSpringSupportFactory.getValidUsers().getUser(memberNumber);
                if(user != null) {
                    m_UserMap.put(user.getUserName(), new Date());
                }
            }

            if(conf == null) return true;

            if(action.equalsIgnoreCase("del-member")) {
                ProcessConfUserDel(conf, conf.get(memberId));
                conf.delete(memberId);

                m_UserMap.remove(memberNumber);

                if(confSize.equals("0")) {
                    // last one to leave the conference
                    ProcessConfEnd(event, conf);
                    m_ConferenceMap.remove(confName);
                }
            }

            if(action.equalsIgnoreCase("start-talking")) {
                conf.setLastMemberTalking(memberId);
                conf.setNoOneTalking(false);
            }

            if(action.equalsIgnoreCase("stop-talking")) {
                if(conf.getWhosTalking().equals(memberId)) {
                    conf.setNoOneTalking(true);
                }
            }

            if(action.equalsIgnoreCase("mute-member") || action.equalsIgnoreCase("unmute-member")) {
                ConferenceMember member = conf.get(event.getEventValue("member-id"));
                if(member != null) {
                    member.m_muted = action.equalsIgnoreCase("mute-member");
                }
            }

            if(action.equalsIgnoreCase("lock") || action.equalsIgnoreCase("unlock")) {
                conf.setLocked(action.equalsIgnoreCase("lock"));
            }
            return true;
        }
        return false;
    }

    // if user in a conference returns date when user entered conference otherwise
    // returns null
    public static Date inConferenceSince(String UserName) {
        return m_UserMap.get(UserName);
    }

    public static Collection<ConferenceMember> getMembers(String confName) {
        ConferenceTask conf = m_ConferenceMap.get(confName);
        if(conf == null) {
            return null;
        } else {
            return conf.getMembers();
        }
    }

    public static ConferenceMember getMember(String confName, String memberId) {
        ConferenceTask conf = m_ConferenceMap.get(confName);
        if(conf == null) {
            return null;
        } else {
            return conf.get(memberId);
        }
    }

    public static boolean isLocked(String confName) {
        ConferenceTask conf = m_ConferenceMap.get(confName);
        if(conf == null) {
            return false;
        } else {
            return conf.isLocked();
        }
    }

    public static String getWhosTalking(String confName) {
        ConferenceTask conf = m_ConferenceMap.get(confName);
        if(conf == null) {
            return null;
        } else {
            return conf.getWhosTalking();
        }
    }

    public static boolean isNoOneTalking(String confName) {
        ConferenceTask conf = m_ConferenceMap.get(confName);
        if(conf == null) {
            return true;
        } else {
            return conf.isNoOneTalking();
        }
    }

    private Socket getSocket() {
        Socket socket = null;

        while(socket == null) {
            // freeswitch may be slow to start especially on a different machine
            try {
                socket = new Socket("localhost", Integer.parseInt(fsListenPort));
            } catch (UnknownHostException e) {
                LOG.error("Can't create connection Conference Server " + e.getMessage());
            } catch (IOException e) {
                // freeswitch likely is not up yet
                try {
                    sleep(4000);
                } catch (InterruptedException e1) {

                }
            }
        }
        return socket;
    }

    @Override
    public void run() {
        try{
            UnfortunateLackOfSpringSupportFactory.initialize();
        } catch (UnknownHostException e) {
            LOG.error(e);
        }
        // make a socket connection to freeswitch
        m_fsListenSocket = new FreeSwitchEventSocket(m_fsConfig);
        m_fsCmdSocket = new FreeSwitchEventSocket(m_fsConfig);
        FreeSwitchEvent event;

        for (;;) {
            Socket commandSocket = getSocket();
            Socket listenSocket = getSocket();

            LOG.info("ConfBasicThread::run() connecting");

            try {
                m_fsCmdSocket.connect(commandSocket, fsPassword);

                if (m_fsListenSocket.connect(listenSocket, fsPassword)) {
                    MonitorConf monConf = new MonitorConf(m_fsListenSocket);
                    monConf.go();

                    for(;;) {
                        event = m_fsListenSocket.awaitEvent();
                        if (!processEvent(event)) {
                            // This happens when FS is restarted
                            LOG.info("ConfBasicThread::run() disconnecting");
                            break;
                        }
                    }
                }
                else {
                    LOG.error("ConfBasicThread::run() failed to connect");
                }
            } catch (Exception e) {
                LOG.error("ConfBasicThread::run() got Exception: " + e.getMessage());
            }

            // Close any open sockets
            finally {
                try {
                    m_fsCmdSocket.close();
                    m_fsListenSocket.close();
                } catch (IOException e) {
                    LOG.error("ConfBasicThread::run() got IOException when closing sockets: " + e.getMessage());
                }
            }

            // Delay before attempting to connect again.
            try {
                sleep(1000);
            } catch (InterruptedException e1) {
            }
        }
    }
}
