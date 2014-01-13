/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.springframework.security.authentication.ProviderManager;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.AuthenticationException;

public class ProvisioningContextImpl implements ProvisioningContext {
    private CoreContext m_sipxCoreContext;
    private ProviderManager m_authManager;
    private PhoneContext m_sipxPhoneContext;
    private Upload m_sipxUpload;

    public void setSipxCoreContext(CoreContext sipxCoreContext) {
        m_sipxCoreContext = sipxCoreContext;
    }

    public void setSipxPhoneContext(PhoneContext sipxPhoneContext) {
        m_sipxPhoneContext = sipxPhoneContext;
    }

    public void setSipxUpload(Upload sipxUpload) {
        m_sipxUpload = sipxUpload;
    }

    public void setAuthManager(ProviderManager manager) {
        m_authManager = manager;
    }

    /**
     * Returns user if credentials check out. Return null if the user does not exist or the
     * password is wrong.
     */
    @Override
    public User getUser(String username, String password) {
        User user = m_sipxCoreContext.loadUserByUserNameOrAlias(username);
        if (user != null) {
            if (checkLogin(user, password)) {
                return user;
            }
        }
        return null;
    }

    @Override
    public String getDomainName() {
        return m_sipxCoreContext.getDomainName();
    }

    @Override
    public String getUploadDirectory() {
        return m_sipxUpload.getDestinationDirectory();
    }

    public CoreContext getSipxCoreContext() {
        return m_sipxCoreContext;
    }

    public PhoneContext getSipxPhoneContext() {
        return m_sipxPhoneContext;
    }

    public Upload getSipxUpload() {
        return m_sipxUpload;
    }

    @Override
    public String getConfDir() {
        return m_sipxPhoneContext.getSystemDirectory();
    }

    private boolean checkLogin(User user, String password) {
        try {
            Authentication auth = new UsernamePasswordAuthenticationToken(user.getUserName(), password);
            m_authManager.authenticate(auth);
            return true;
        } catch (AuthenticationException exception) {
            return false;
        }
    }

    @Override
    public Phone getPhoneForUser(User user, String[] phoneModels) {
        for (String phoneModel : phoneModels) {
            Collection<Phone> phones = getSipxPhoneContext()
                    .getPhonesByUserIdAndPhoneModel(user.getId(), phoneModel);
            for (Phone phone : phones) {
                if (phone.getLine(0).getUser().equals(user)) {
                    return phone;
                }
            }
        }
        return null;
    }
}
