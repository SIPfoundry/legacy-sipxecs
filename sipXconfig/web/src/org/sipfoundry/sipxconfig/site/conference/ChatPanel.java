/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.contrib.link.PopupLinkRenderer;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ChatConference;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;


public abstract class ChatPanel extends BaseComponent implements PageBeginRenderListener {
    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Bean
    public abstract PopupLinkRenderer getPopupRenderer();

    @Parameter(required = true)
    public abstract Conference getConference();

    @Parameter(required = true)
    public abstract SipxValidationDelegate getValidator();

    @Parameter
    public abstract ICallback getCallback();

    public abstract ChatConference getChatConference();

    public abstract void setChatConference(ChatConference chat);

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getChatConference() == null) {
            setChatConference(new ChatConference(getConference()));
        }
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            getConferenceBridgeContext().store(getConference());
        }
    }
}
