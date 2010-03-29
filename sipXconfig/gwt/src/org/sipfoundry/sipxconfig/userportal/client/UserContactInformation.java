/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.user.client.Timer;
import com.smartgwt.client.types.Alignment;
import com.smartgwt.client.widgets.IButton;
import com.smartgwt.client.widgets.Img;
import com.smartgwt.client.widgets.events.ClickEvent;
import com.smartgwt.client.widgets.events.ClickHandler;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.ValuesManager;
import com.smartgwt.client.widgets.form.fields.BooleanItem;
import com.smartgwt.client.widgets.form.fields.events.ChangeEvent;
import com.smartgwt.client.widgets.form.fields.events.ChangeHandler;
import com.smartgwt.client.widgets.layout.HLayout;
import com.smartgwt.client.widgets.layout.VLayout;
import com.smartgwt.client.widgets.tab.Tab;
import com.smartgwt.client.widgets.tab.TabSet;

import org.sipfoundry.sipxconfig.userportal.locale.ContactInformationConstants;
import org.sipfoundry.sipxconfig.userportal.widget.ContactInformationDataSource;


public class UserContactInformation implements EntryPoint {

    private static ContactInformationConstants s_constants = GWT.create(ContactInformationConstants.class);

    private ValuesManager m_valuesManager;
    private Avatar m_avatar;
    private ContactInformationForm m_officeAddressForm;
    private final ContactInformationDataSource m_dataSource = new ContactInformationDataSource("contactData");

    @Override
    public void onModuleLoad() {

        ContactInformationForm generalForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_GENERAL,
                "generalForm", 2, s_constants);
        ContactInformationForm homeForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_HOME,
                "homeForm", 2, s_constants);
        m_officeAddressForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_OFFICE,
                "officeForm", 2, s_constants);

        generalForm.addEmailValidator(ContactInformationDataSource.EMAIL_ADDRESS);
        generalForm.addEmailValidator(ContactInformationDataSource.ALTERNATE_EMAIL_ADDRESS);
        generalForm.getItem(ContactInformationDataSource.IM_ID).setDisabled(true);

        m_valuesManager = new ValuesManager();
        m_valuesManager.setDataSource(m_dataSource);
        m_valuesManager.fetchData();

        generalForm.setValuesManager(m_valuesManager);
        homeForm.setValuesManager(m_valuesManager);
        m_officeAddressForm.setValuesManager(m_valuesManager);

        BranchAddressForm useBranchAddressForm = new BranchAddressForm(m_officeAddressForm, m_valuesManager);
        m_avatar = new Avatar();

        VLayout generalLayout = new VLayout();
        generalLayout.addMember(m_avatar);
        generalLayout.addMember(generalForm);

        VLayout addressLayout = new VLayout();
        addressLayout.addMember(homeForm);
        addressLayout.addMember(m_officeAddressForm);
        addressLayout.addMember(useBranchAddressForm);

        TabSet contactTabSet = new TabSet();
        Tab generalTab = new Tab();
        generalTab.setPane(generalLayout);
        generalTab.setTitle(s_constants.generalForm());
        Tab addressTab = new Tab();
        addressTab.setTitle(s_constants.address());
        addressTab.setPane(addressLayout);
        contactTabSet.setTabs(generalTab, addressTab);
        contactTabSet.setHeight100();
        contactTabSet.setWidth100();

        VLayout contactInformationLayout = new VLayout();
        contactInformationLayout.addMember(contactTabSet);
        contactInformationLayout.addMember(getSaveButton());
        contactInformationLayout.setHeight100();
        contactInformationLayout.setWidth("62%");

        contactInformationLayout.draw();
        initRecordsWithDelay();
    }

    private HLayout getSaveButton() {
        IButton saveButton = new IButton(s_constants.save());
        saveButton.addClickHandler(new ClickHandler() {
            @Override
            public void onClick(ClickEvent event) {
                if (m_valuesManager.validate() && m_valuesManager.valuesHaveChanged()) {
                    m_dataSource.editEntry(m_valuesManager);
                    refreshRecordsWithDelay();
                }
            }
        });

        HLayout saveButtonLayout = new HLayout();
        saveButtonLayout.addMember(saveButton);
        saveButtonLayout.setPadding(10);

        return saveButtonLayout;
    }

    private void refreshRecordsWithDelay() {
        // fetch data with 1.5 secs delay so the changes to be picked up
        Timer refreshTimer = new Timer() {
            @Override
            public void run() {
                m_valuesManager.fetchData();
                initRecordsWithDelay();
            }
        };
        refreshTimer.schedule(1500);
    }

    private void initRecordsWithDelay() {
        // fetch data with 1.5 secs delay so the changes to be picked up
        Timer refreshTimer = new Timer() {
            @Override
            public void run() {
                m_avatar.update();
                Boolean useBranchAddress = Boolean.valueOf(m_valuesManager.getValueAsString(
                        ContactInformationDataSource.USE_BRANCH_ADDRESS));
                if (useBranchAddress) {
                    m_officeAddressForm.maskData();
                } else {
                    m_officeAddressForm.unMaskData();
                }
            }
        };
        refreshTimer.schedule(1500);
    }

    private class BranchAddressForm extends DynamicForm {

        public BranchAddressForm(final ContactInformationForm officeAddressForm, ValuesManager valuesManager) {

            setValuesManager(m_valuesManager);

            BooleanItem useBranchAddress = new BooleanItem();
            useBranchAddress.setDataPath(ContactInformationDataSource.USE_BRANCH_ADDRESS);
            useBranchAddress.setTitle(s_constants.getString(ContactInformationDataSource.USE_BRANCH_ADDRESS));
            useBranchAddress.setTitleStyle("titleFormStyle");
            useBranchAddress.addChangeHandler(new ChangeHandler() {
                @Override
                public void onChange(ChangeEvent event) {
                    Boolean useBranch = (Boolean) event.getValue();
                    ValuesManager vm = m_officeAddressForm.getValuesManager();
                    if (useBranch) {
                        m_officeAddressForm.maskData();
                    } else {
                        m_officeAddressForm.unMaskData();
                    }
                }
            });
            setItems(useBranchAddress);
        }
    }

    private class Avatar extends Img {

        public Avatar() {
            setStyleName("gwtAvatarImg");
            setWidth(140);
            setHeight(140);
            setLayoutAlign(Alignment.CENTER);
        }

        public void update() {
            setSrc(m_valuesManager.getValueAsString(ContactInformationDataSource.AVATAR));
        }
    }
}
