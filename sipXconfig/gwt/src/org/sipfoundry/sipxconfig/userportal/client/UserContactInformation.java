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
import com.smartgwt.client.widgets.IButton;
import com.smartgwt.client.widgets.events.ClickEvent;
import com.smartgwt.client.widgets.events.ClickHandler;
import com.smartgwt.client.widgets.form.ValuesManager;
import com.smartgwt.client.widgets.layout.HLayout;
import com.smartgwt.client.widgets.layout.VLayout;

import org.sipfoundry.sipxconfig.userportal.locale.ContactInformationConstants;
import org.sipfoundry.sipxconfig.userportal.widget.ContactInformationDataSource;


public class UserContactInformation implements EntryPoint {

    private static ContactInformationConstants s_constants = GWT.create(ContactInformationConstants.class);

    private ValuesManager m_valuesManager;
    private final ContactInformationDataSource m_dataSource = new ContactInformationDataSource("contactData");

    @Override
    public void onModuleLoad() {

        ContactInformationForm generalForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_GENERAL,
                "generalForm", 2, s_constants);
        ContactInformationForm homeForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_HOME,
                "homeForm", 2, s_constants);
        ContactInformationForm officeForm = new ContactInformationForm(ContactInformationDataSource.FIELDS_OFFICE,
                "officeForm", 2, s_constants);

        generalForm.addEmailValidator(ContactInformationDataSource.EMAIL_ADDRESS);
        generalForm.addEmailValidator(ContactInformationDataSource.ALTERNATE_EMAIL_ADDRESS);

        m_valuesManager = new ValuesManager();
        m_valuesManager.setDataSource(m_dataSource);
        m_valuesManager.fetchData();

        generalForm.setValuesManager(m_valuesManager);
        homeForm.setValuesManager(m_valuesManager);
        officeForm.setValuesManager(m_valuesManager);

        VLayout layout = new VLayout();
        layout.addMember(generalForm);
        layout.addMember(homeForm);
        layout.addMember(officeForm);
        layout.addMember(getSaveButton());

        layout.draw();
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
            }
        };
        refreshTimer.schedule(1500);
    }
}
