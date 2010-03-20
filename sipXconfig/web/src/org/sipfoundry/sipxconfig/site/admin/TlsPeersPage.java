/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class TlsPeersPage extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/TlsPeers";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectPage(ManageCertificates.PAGE)
    public abstract ManageCertificates getManageCertificates();

    @InjectPage(EditTlsPeer.PAGE)
    public abstract EditTlsPeer getEditTlsPeerPage();

    @InjectObject(value = "spring:tlsPeerManager")
    public abstract TlsPeerManager getTlsPeerManager();

    @Bean
    public abstract SelectMap getSelections();

    public abstract TlsPeer getCurrentRow();

    public void pageBeginRender(PageEvent event) {

    }

    public IPage addTlsPeer(IRequestCycle cycle) {
        return getEditTlsPeerPage(null);
    }

    public IPage editTlsPeer(int peerId) {
        return getEditTlsPeerPage(peerId);
    }

    private IPage getEditTlsPeerPage(Integer peerId) {
        EditTlsPeer page = getEditTlsPeerPage();
        page.setTlsPeerId(peerId);
        page.setReturnPage(PAGE);
        return page;
    }

    public IPage certificateAuthorities() {
        ManageCertificates page = getManageCertificates();
        page.setTab(ManageCertificates.CERT_AUTH_TAB);
        return page;
    }

    public TlsPeersTableModel getTlsPeersModel() {
        TlsPeersTableModel tableModel = new TlsPeersTableModel();
        List<TlsPeer> list = getTlsPeerManager().getTlsPeers();
        tableModel.setTlsPeers(list);
        return tableModel;
    }

    public void deleteTlsPeers() {
        Collection<Integer> selectedLocations = getSelections().getAllSelected();
        getTlsPeerManager().deleteTlsPeers(selectedLocations);
    }

}
