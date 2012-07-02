package org.sipfoundry.sipximbot;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.ConfBasicThread;
import org.sipfoundry.commons.freeswitch.ConfCommand;
import org.sipfoundry.commons.userdb.User;

public class ConfTask extends ConfBasicThread {

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipximbot");
    static Localizer m_localizer;

    ConfTask() {
        m_localizer = new Localizer();
    }

    public static synchronized String ConfCommand(User user, String cmd, Localizer localizer) {

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
