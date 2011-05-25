/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

/* ProfileContext for NortelPhonebook is created */
/* when NortelPhone's getPhonebook method is called */

public class NortelPhonebook extends ProfileContext {

    private Collection<PhonebookEntry> m_phonebook;
    private Collection<NortelPhonebookEntry> m_nortelPhonebook = new ArrayList<NortelPhonebookEntry>();
    private CoreContext m_coreContext;

    /**
     * @param phonebook is a collection of PhonebookEntry
     * @param coreContext provides access to user DAO(or servicelayer)
     */
    public NortelPhonebook(Collection<PhonebookEntry> phonebook, CoreContext coreContext) {
//      sets the velocity templates to be used for profile generation
        super(null, "nortel/mac-phonebook.cfg.vm");
        m_phonebook = phonebook;
        m_coreContext = coreContext;
    }

    public Collection<NortelPhonebookEntry> getPhonebook() {
        return m_nortelPhonebook;
    }
     /**
     * for setting phonebooks created without the UI
     * @param nortelPhonebook
     */
    public void setPhonebook(Collection<NortelPhonebookEntry> nortelPhonebook) {
        m_nortelPhonebook = nortelPhonebook;
    }
    /**
     * return Map (all the settings which are used by the context , basically
     * These are the object names used in the velocity templates)
     */
    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        if (m_phonebook != null) {

            for (PhonebookEntry entry : m_phonebook) {
                NortelPhonebookEntry nortelPhonebookEntry = new NortelPhonebookEntry();
                /* creating a NortelPhonebookEntry against each PhonebookEntry */
                String number = entry.getNumber();
                User user = m_coreContext.loadUserByUserNameOrAlias(number);

                if (user != null) {
                    String uri = null;
                    String nickName = null;
                    if (user.getFirstName() != null && user.getLastName() != null) {
                        nickName = user.getFirstName() + " " + user.getLastName();
                        /*if user has First Name and Last Name then NickName = FN and LN
                         * else its equal to UserName
                         */
                    } else {
                        nickName = user.getUserName();
                    }
                    String fullSipUri = user.getAddrSpec(m_coreContext.getDomainName());
                    Integer index = fullSipUri.indexOf(":");
                    /*nortel phones require a sip uri without "sip:"*/
                    uri = fullSipUri.substring(index + 1);
                    nortelPhonebookEntry.setSipUri(uri);
                    nortelPhonebookEntry.setNickName(nickName);
                    nortelPhonebookEntry.setNumber(number);
                    m_nortelPhonebook.add(nortelPhonebookEntry);
                }


            }
            //phonebook object is used in the velocity templates
            context.put("phonebook", m_nortelPhonebook);
            context.put("phonebookVersionNo", System.currentTimeMillis());

        }
        return context;
    }

    /**
     * public so Velocity doesn't reject object
     * NortelPhonebookEntry requires an extra parameter sipUri
     */
    public static class NortelPhonebookEntry extends PhonebookEntry {
        private String m_sipUri;
        private String m_nickName;

        public String getSipUri() {
            return m_sipUri;
        }
        public void setSipUri(String sipUri) {
            m_sipUri = sipUri;
        }
        public String getNickName() {
            return m_nickName;
        }
        public void setNickName(String nickName) {
            m_nickName = nickName;
        }
    }
}
