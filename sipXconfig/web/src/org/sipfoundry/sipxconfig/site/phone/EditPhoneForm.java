/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.translator.Translator;
import org.apache.tapestry.form.validator.Validator;
import org.sipfoundry.sipxconfig.components.SerialNumberTranslator;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class EditPhoneForm extends BaseComponent {
    @Parameter(required = true)
    public abstract Phone getPhone();

    @Parameter(required = true)
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    public abstract String getGroupsString();

    public abstract void setGroupsString(String groupsString);

    public abstract Collection getGroupsCandidates();

    public abstract void setGroupCandidates(Collection groupsList);

    public void buildGroupCandidates(String groupsString) {
        List allGroups = getPhoneContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {

        if (!TapestryUtils.isRewinding(cycle, this)) {
            if (getGroupsString() == null) {
                Phone phone = getPhone();
                setGroupsString(phone.getGroupsNames());
            }
        }

        super.renderComponent(writer, cycle);

        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(Phone.GROUP_RESOURCE_ID, groupsString, false);
                Phone phone = getPhone();
                phone.setGroupsAsList(groups);
            }
        }
    }

    public List<Validator> getSerialNumberValidators() {
        return TapestryUtils.getSerialNumberValidators(getPhone().getModel());
    }

    public Translator getSerialNumberTranslator() {
        return new SerialNumberTranslator(getPhone().getModel());
    }
}
