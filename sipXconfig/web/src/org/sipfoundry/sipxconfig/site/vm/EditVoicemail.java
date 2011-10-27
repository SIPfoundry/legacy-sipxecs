/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;

public abstract class EditVoicemail extends UserBasePage {

    public static final String PAGE = "vm/EditVoicemail";

    @Persist
    public abstract void setVoicemail(Voicemail voicemail);
    public abstract Voicemail getVoicemail();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public void save() {
        if (!getValidator().getHasErrors()) {
            getMailboxManager().save(getVoicemail());
        }
    }

}
