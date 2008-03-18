/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class UserPhones extends UserBasePage {
    public static final String PAGE = "user/UserPhones";

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getPhoneProfileManager();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Collection<Integer> getGenerateProfileIds();

    public Collection<Phone> getPhones() {
        return getPhoneContext().getPhonesByUserId(getUserId());
    }
    
    public IPage addExistingPhones(IRequestCycle cycle) {
        AddExistingPhone page = (AddExistingPhone) cycle.getPage(AddExistingPhone.PAGE);
        page.setUserId(getUser().getId());
        page.setReturnPage(PAGE);
        return page;
    }
}
