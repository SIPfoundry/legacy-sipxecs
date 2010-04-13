/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditTlsPeer extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "admin/EditTlsPeer";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:tlsPeerManager")
    public abstract TlsPeerManager getTlsPeerManager();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    public abstract TlsPeer getTlsPeer();

    public abstract void setTlsPeer(TlsPeer peer);

    @Persist
    public abstract Integer getTlsPeerId();

    public abstract void setTlsPeerId(Integer peerId);

    public void pageBeginRender(PageEvent event) {
        TlsPeer peer = getTlsPeer();
        if (peer != null) {
            if (!peer.isNew()) {
                setTlsPeerId(peer.getId());
            }
            return;
        }
        if (getTlsPeerId() != null) {
            peer = getTlsPeerManager().getTlsPeer(getTlsPeerId());
        } else {
            peer = getTlsPeerManager().newTlsPeer();
        }
        setTlsPeer(peer);
    }

    public void saveTlsPeer() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        TlsPeer peer = getTlsPeer();
        getTlsPeerManager().saveTlsPeer(peer);
        setTlsPeerId(peer.getId());
    }

}
