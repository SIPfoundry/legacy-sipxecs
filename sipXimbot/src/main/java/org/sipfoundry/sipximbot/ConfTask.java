package org.sipfoundry.sipximbot;

import java.util.Date;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.ConferenceMember;
import org.sipfoundry.commons.freeswitch.ConferenceTask;
import org.sipfoundry.commons.freeswitch.FreeSwitchEvent;
import org.sipfoundry.commons.freeswitch.ConfCommand;

public class ConfTask extends ConfBasicThread {

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    static Localizer m_localizer;

    ConfTask() {
        m_localizer = new Localizer();
    }

    public void ProcessConfStart(FreeSwitchEvent event, ConferenceTask conf) {
        FullUsers users = FullUsers.update();
        FullUser owner = users.findByConfName(event.getEventValue("conference-name"));
        if(owner != null) {
            conf.setOwner(owner);
        }
    }

    public void ProcessConfEnd(FreeSwitchEvent event, ConferenceTask conf) {
    }

    public void ProcessConfUserAdd(ConferenceTask conf, ConferenceMember member) {

        if(conf.getOwner() == null) {
            return;
        }

        FullUsers users = FullUsers.update();
        FullUser owner = users.isValidUser(conf.getOwner().getUserName());

        if(owner.getConfEntryIM()) {
            Date date = new Date();

            IMBot.sendIM(conf.getOwner().getUserName(), member.memberName() + " (" + member.memberNumber() + ") " +
                         m_localizer.localize("participant_entered") + " [" + member.memberIndex() + "] at " + date.toString());
        }
    }

    public void ProcessConfUserDel(ConferenceTask conf, ConferenceMember member) {

        if(conf.getOwner() == null) {
            return;
        }

        FullUsers users = FullUsers.update();
        FullUser owner = users.isValidUser(conf.getOwner().getUserName());

        if(owner.getConfExitIM()) {
            Date date = new Date();

            IMBot.sendIM(conf.getOwner().getUserName(), member.memberName() + " (" + member.memberNumber() + ") " +
                         m_localizer.localize("participant_left") + " " + date.toString());
        }
    }

    public static synchronized String ConfCommand(FullUser user, String cmd, Localizer localizer) {

        if(user.getConfName() == null) {
            return m_localizer.localize("no_conf");
        }

        ConfCommand confcmd = new ConfCommand(ConfBasicThread.getCmdSocket(), user.getConfName(), cmd, localizer);
        confcmd.go();

        if (confcmd.isSucessful()) {
            return null;
        } else {
            LOG.debug("Conf command " + cmd + " " + confcmd.GetErrString());
            return confcmd.GetErrString();
        }
    }

}
