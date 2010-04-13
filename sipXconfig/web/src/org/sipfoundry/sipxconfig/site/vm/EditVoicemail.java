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

import java.io.File;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.VoicemailSource;

public abstract class EditVoicemail extends UserBasePage {

    public static final String PAGE = "vm/EditVoicemail";

    @Persist(value = "client")
    public abstract void setVoicemailId(String voicemailId);
    public abstract String getVoicemailId();

    public abstract Voicemail getVoicemail();
    public abstract void setVoicemail(Voicemail voicemail);

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    public abstract VoicemailSource getVoicemailSource();
    public abstract void setVoicemailSource(VoicemailSource source);

    public void save() {
        if (!getValidator().getHasErrors()) {
            getVoicemail().save();
        }
    }

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        VoicemailSource source = getVoicemailSource();
        if (source == null) {
            source = new VoicemailSource(new File(getMailboxManager().getMailstoreDirectory()));
            setVoicemailSource(source);
        }

        Voicemail vm = getVoicemail();
        if (vm == null) {
            vm = source.getVoicemail(getVoicemailId());
            setVoicemail(vm);
        }
    }
}
