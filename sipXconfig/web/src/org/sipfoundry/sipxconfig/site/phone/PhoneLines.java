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

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.line.AddExternalLine;
import org.sipfoundry.sipxconfig.site.line.EditLine;

/**
 * Manage a phone's lines
 */
public abstract class PhoneLines extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "phone/PhoneLines";

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectPage(value = AddPhoneUser.PAGE)
    public abstract AddPhoneUser getAddPhoneUserPage();

    @InjectPage(value = AddExternalLine.PAGE)
    public abstract AddExternalLine getAddExternalLinePage();

    @InjectPage(value = EditLine.PAGE)
    public abstract EditLine getEditLinePage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    @Persist
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    public Collection getLines() {
        return getPhone().getLines();
    }

    public abstract Line getCurrentRow();

    public abstract void setCurrentRow(Line line);

    public abstract int getCurrentIndex();

    @InitialValue(value = "new org.sipfoundry.sipxconfig.components.SelectMap()")
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    public abstract String getActiveTab();

    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        Phone phone = context.loadPhone(getPhoneId());
        setPhone(phone);
    }

    public IPage addExternalLine(Integer phoneId) {
        checkMaxLines(phoneId);
        AddExternalLine page = getAddExternalLinePage();
        page.setPhoneId(phoneId);
        page.setReturnPage(this);
        return page;
    }

    private void checkMaxLines(Integer phoneId) {
        Phone phone = getPhoneContext().loadPhone(phoneId);
        phone.addLine(phone.createLine());
    }

    public IPage addLine(Integer phoneId) {
        checkMaxLines(phoneId);
        AddPhoneUser page = getAddPhoneUserPage();
        page.setPhoneId(phoneId);
        return page;
    }

    public IPage editLine(Integer lineId) {
        EditLine page = getEditLinePage();
        page.setLineId(lineId);
        return page;
    }

    public void deleteLine() {
        PhoneContext context = getPhoneContext();
        Phone phone = getPhone();

        // hack, avoid hibernate exception on unsaved valuestorage objects
        // on orphaned lines
        context.storePhone(phone);

        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.removeByPrimaryKey(phone.getLines(), lineIds);

        context.storePhone(phone);
    }

    public void moveLineUp() {
        moveLines(-1);
    }

    public void moveLineDown() {
        moveLines(1);
    }

    private void moveLines(int step) {
        PhoneContext context = getPhoneContext();
        Phone phone = getPhone();
        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.moveByPrimaryKey(phone.getLines(), lineIds, step);
        context.storePhone(phone);
    }

    public String ok() {
        apply();
        return ManagePhones.PAGE;
    }

    public void apply() {
        PhoneContext dao = getPhoneContext();
        dao.storePhone(getPhone());
        dao.flush();
    }

    public String cancel() {
        return ManagePhones.PAGE;
    }

    public boolean isExternalLinesSupported() {
        return getPhone().getModel().isExternalLinesSupported();
    }

    public int getMaxLineCount() {
        return getPhone().getModel().getMaxLineCount();
    }
}
