/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.update;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.ContactInfoChangeApi;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.springframework.beans.factory.annotation.Required;

public class XmppContactInformationUpdate {
    private ApiProvider<ContactInfoChangeApi> m_contactInfoChangeApiProvider;
    private AddressManager m_addressManager;

    @Required
    public void setContactInfoChangeApiProvider(ApiProvider<ContactInfoChangeApi> contactInfoChangeApiProvider) {
        m_contactInfoChangeApiProvider = contactInfoChangeApiProvider;
    }

    public void notifyChange(User user) {
        ImAccount imAccount = new ImAccount(user);
        if (imAccount.isEnabled()) {
            getApi().notifyContactChange(imAccount.getImId());
        }
    }

    private ContactInfoChangeApi getApi() {
        Address imApi = m_addressManager.getSingleAddress(ImManager.XMLRPC_VCARD_ADDRESS);
        return m_contactInfoChangeApiProvider.getApi(imApi.toString());
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }
}
