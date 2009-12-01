/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.moh;

import java.io.File;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Locale;
import java.util.ResourceBundle;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchEventSocketInterface;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.Sleep;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;


public class Moh {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    // Global store for AutoAttendant resource bundles keyed by locale
    private static final String RESOURCE_NAME="org.sipfoundry.attendant.AutoAttendant";
    private static HashMap<Locale, ResourceBundle> s_resourcesByLocale = new HashMap<Locale, ResourceBundle>();

    private IvrConfiguration m_ivrConfig;
    private FreeSwitchEventSocketInterface m_fses;
    private String m_mohParam;
    private ValidUsersXML m_validUsers;
    @SuppressWarnings("unused")
    private TextToPrompts m_ttp;
    private String m_localeString;
    private Localization m_loc;

    enum NextAction {
        repeat, exit, nextAttendant;
    }

    /**
     * Create an Moh.
     * 
     * @param ivrConfig top level configuration stuff
     * @param fses The FreeSwitchEventSocket with the call already answered
     * @param parameters The parameters from the sip URI (to determine locale and which Moh
     *        id to use)
     */
    public Moh(IvrConfiguration ivrConfig, FreeSwitchEventSocketInterface fses,
            Hashtable<String, String> parameters) {
        this.m_ivrConfig = ivrConfig;
        this.m_fses = fses;
        this.m_mohParam = parameters.get("moh");

        // Look for "locale" parameter
        m_localeString = parameters.get("locale");
        if (m_localeString == null) {
            // Okay, try "lang" instead
            m_localeString = parameters.get("lang");
        }
    }

    /**
     * Load all the needed configuration.
     * 
     * The attendantBundle with the resources is located based on locale, as is the TextToPrompts
     * class. The attendant configuration files are loaded (if they changed since last time), and
     * the ValidUsers (also if they changed).
     *      * 
     */
    void loadConfig() {
        // Load the resources for the given locale.
        m_loc = new Localization(RESOURCE_NAME, 
                m_localeString, s_resourcesByLocale, m_ivrConfig, m_fses);
        
        // Update the valid users list
        try {
            m_validUsers = ValidUsersXML.update(LOG, true);
        } catch (Exception e) {
            System.exit(1); // If you can't trust validUsers, who can you trust?
        }
    }


    /**
     * Run each Moh until there is nothing left to do. If the SIP URL didn't pass in a
     * particular attendant name, use the current time of day and the schedule to find which
     * attendant to run.
     * 
     * Keep running the next returned attendant until there are none left, then exit.
     * 
     * @throws Throwable indicating an error or hangup condition.
     */
    public void run() {

        String id = null;

        if (m_loc == null) {
            loadConfig();
        }

        if (m_mohParam == null || m_mohParam.equals("")) {
            id = "";
        } else {
            id = m_mohParam;
            LOG.info(String.format("Moh::run Moh %s determined from URL parameter", id));
        }

        // Wait it bit so audio doesn't start too fast
        Sleep s = new Sleep(m_fses, 1000);
        s.go();

        moh(id);
    }

    /**
     * Do the specified Moh.
     * 
     * @param id The id of the moh.
     * <br><pre>     id       meaning
     * -------  ----------
     * (empty)  use original park server music (/var/sipxdata/parkserver/music/)
     * l        use local_stream://moh (defined in local_stream.conf.xml)
     * p        use portaudio_stream:// (defined in portaudio.conf.xml)
     * u{user}  use per user music ({data}/moh/{username})
     *</pre>
     */
    void moh(String id){
        LOG.info("Moh::moh Starting moh id (" + id + ") in locale " + m_loc.getLocale());

        PromptList pl = m_loc.getPromptList();
        String musicPath;
        if (id.equals("l")) {
            musicPath = "local_stream://moh";
        } else if (id.equals("p")) {
            musicPath = "portaudio_stream://";
        } else if (id.startsWith("u")) {
            String userName = id.substring(1);
            User user = m_validUsers.getUser(userName);
            if (user != null) {
                musicPath = m_ivrConfig.getDataDirectory()+"/moh/"+user.getUserName();
            } else {
                // Use default FreeSWITCH MOH
                musicPath = "local_stream://moh";
            }
        } else {
            // Use original park server music
            musicPath = m_ivrConfig.getPromptsDirectory()+"/../../../parkserver/music/";
        }
        if (musicPath.contains("://")) {
            LOG.info("Moh::moh Using MOH URL "+musicPath);
            // musicPath is a URL (that FreeSWITCH knows how to deal with)
            pl.addUrl(musicPath);
        } else {
            // musicPath is a file or directory
            File musicPathFile = new File(musicPath);
            if (musicPathFile.isFile()) {
                LOG.info("Moh::moh Using MOH File "+musicPath);
                // musicPath is a file  Use it.
                pl.addPrompts(musicPath);
            } else if (musicPathFile.isDirectory()) {
                LOG.info("Moh::moh Using MOH Directory "+musicPath);
                // musicPath is a directory.  Find all the files inside,
                // sort alphabetically, and use them.
                File[] musicFiles = musicPathFile.listFiles();
                Arrays.sort(musicFiles);
                for (File musicFile : musicFiles) {
                    pl.addPrompts(musicFile.getPath());
                }
            } else {
                LOG.warn("Moh::moh MOH path unknown "+musicPath);
                // Oops.  Something not found, use default FS MOH
                pl.addUrl("local_stream://moh");
            }
        }
        // Play the music until someone hangs up.
        // (FreeSWITCH will hang up after 300 seconds with no RTP)
        for(;;) {
            m_loc.play(pl, "");
            Sleep s = new Sleep(m_fses, 1000);
            s.go();
        }
    }

 
}
