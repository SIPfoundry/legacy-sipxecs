/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import java.util.Arrays;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.i18n.client.ConstantsWithLookup;
import com.google.gwt.user.client.ui.RootPanel;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.types.OperatorId;
import com.smartgwt.client.types.Overflow;
import com.smartgwt.client.types.Side;
import com.smartgwt.client.widgets.Img;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.fields.events.ChangedEvent;
import com.smartgwt.client.widgets.form.fields.events.ChangedHandler;
import com.smartgwt.client.widgets.grid.ListGrid;
import com.smartgwt.client.widgets.grid.ListGridField;
import com.smartgwt.client.widgets.grid.ListGridRecord;
import com.smartgwt.client.widgets.grid.events.RecordClickEvent;
import com.smartgwt.client.widgets.grid.events.RecordClickHandler;
import com.smartgwt.client.widgets.layout.HLayout;
import com.smartgwt.client.widgets.layout.VLayout;
import com.smartgwt.client.widgets.tab.Tab;
import com.smartgwt.client.widgets.tab.TabSet;
import com.smartgwt.client.widgets.viewer.DetailFormatter;
import com.smartgwt.client.widgets.viewer.DetailViewer;
import com.smartgwt.client.widgets.viewer.DetailViewerField;
import com.smartgwt.client.widgets.viewer.DetailViewerRecord;

import org.sipfoundry.sipxconfig.userportal.widget.PhonebookDataSource;

// FIXME: all sizes should be in CSS
public class UserPhonebookSearch implements EntryPoint {
    private static final String FIRST_NAME = "first-name";
    private static final String LAST_NAME = "last-name";
    private static final String NUMBER = "number";
    private static final String EMAIL = "emailAddress";

    private static SearchConstants s_searchConstants = GWT.create(SearchConstants.class);

    @Override
    public void onModuleLoad() {

        DataSource phonebookDS = new PhonebookDataSource("phonebookGridId");
        final Details topTabSet = new Details(s_searchConstants);

        final ListGrid phonebookGrid = new PhonebookGrid(phonebookDS);
        phonebookGrid.addRecordClickHandler(new RecordClickHandler() {
            @Override
            public void onRecordClick(RecordClickEvent event) {
                topTabSet.setData(phonebookGrid.getSelection());
                String number = event.getRecord().getAttribute(NUMBER);
                RootPanel.get("call:number").getElement().setPropertyString("value", number);
            }
        });

        SearchForm searchForm = new SearchForm(phonebookDS);
        searchForm.addHandler(phonebookGrid);

        HLayout hgridLayout = new HLayout();
        hgridLayout.setLayoutRightMargin(5);
        hgridLayout.addMember(phonebookGrid);
        hgridLayout.addMember(topTabSet);
        hgridLayout.setWidth("100%");
        hgridLayout.setHeight(300);

        VLayout gridLayout = new VLayout();
        gridLayout.setLayoutBottomMargin(5);
        gridLayout.addMember(searchForm);
        gridLayout.addMember(hgridLayout);
        gridLayout.setWidth("70%");

        RootPanel.get("user_phonebook_grid").add(gridLayout);
    }

    private static class PhonebookGrid extends ListGrid {
        private static final String NRML_TABLE_FIELD_WIDTH = "20%";

        public PhonebookGrid(DataSource phonebookDS) {
            ListGridField firstNameField = new ListGridField(FIRST_NAME, s_searchConstants.firstName());
            ListGridField lastNameField = new ListGridField(LAST_NAME, s_searchConstants.lastName());
            ListGridField numberField = new ListGridField(NUMBER, s_searchConstants.phoneNumber());
            ListGridField emailField = new ListGridField(EMAIL, s_searchConstants.emailAddress());

            firstNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            lastNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            numberField.setWidth(NRML_TABLE_FIELD_WIDTH);
            emailField.setWidth("*");

            setHeaderTitleStyle("gwtGridHeaderTitle");
            setBaseStyle("gwtGridBody");
            setEmptyMessage(s_searchConstants.noUserFound());
            setWidth("71%");

            setBodyOverflow(Overflow.AUTO);
            setOverflow(Overflow.VISIBLE);
            setAlternateRecordStyles(true);
            setDataSource(phonebookDS);
            fetchData();

            setCellHeight(25);
            setHeaderHeight(35);
            setFields(firstNameField, lastNameField, numberField, emailField);
        }
    }

    private static class SearchForm extends DynamicForm {
        private final TextItem m_firstName;
        private final TextItem m_lastName;
        private final TextItem m_number;
        private final TextItem m_email;

        public SearchForm(DataSource phonebookDS) {
            m_firstName = new TextItem();
            m_firstName.setCriteriaField(FIRST_NAME);
            m_firstName.setOperator(OperatorId.ISTARTS_WITH);
            m_firstName.setTitle(s_searchConstants.searchWidgetTitle());

            m_lastName = new TextItem();
            m_lastName.setCriteriaField(LAST_NAME);
            m_lastName.setOperator(OperatorId.ISTARTS_WITH);
            m_lastName.setVisible(false);

            m_number = new TextItem();
            m_number.setCriteriaField(NUMBER);
            m_number.setOperator(OperatorId.ISTARTS_WITH);
            m_number.setVisible(false);

            m_email = new TextItem();
            m_email.setCriteriaField(EMAIL);
            m_email.setOperator(OperatorId.ISTARTS_WITH);
            m_email.setVisible(false);

            setFields(m_firstName, m_lastName, m_number, m_email);
            setDataSource(phonebookDS);
            setOperator(OperatorId.OR);
        }

        void addHandler(final ListGrid phonebookGrid) {
            ChangedHandler onChange = new ChangedHandler() {
                @Override
                public void onChanged(ChangedEvent event) {
                    m_lastName.setValue((String) m_firstName.getValue());
                    m_number.setValue((String) m_firstName.getValue());
                    m_email.setValue((String) m_firstName.getValue());
                    if (m_firstName.getValue() != null) {
                        phonebookGrid.filterData(getValuesAsCriteria());
                    } else {
                        phonebookGrid.filterData();
                    }
                }
            };
            m_firstName.addChangedHandler(onChange);
        }
    }

    private static class Details extends TabSet {
        private static final String PHONE_CELL = "cellPhoneNumber";
        private static final String PHONE_HOME = "homePhoneNumber";
        private static final String PHONE_ASST = "assistantPhoneNumber";

        private static final String[] FIELDS_GENERAL = {
            "jobTitle", "jobDept", "companyName", "location", PHONE_CELL, "faxNumber", "imId", "alternateImId"
        };

        private static final String[] FIELDS_HOME = {
            "homeStreet", "homeCity", "homeState", "homeCountry", "homeZip", PHONE_HOME
        };

        private static final String[] FIELDS_OFFICE = {
            "officeDesignation", "assistantName", PHONE_ASST, "officeStreet", "officeCity", "officeState",
            "officeCountry", "officeZip"
        };

        private static final String[] FIELDS_PHONENUMBERS = {
            PHONE_CELL, PHONE_HOME, PHONE_ASST
        };

        private final DetailViewer m_phonebookViewerGeneral;
        private final DetailViewer m_phonebookViewerHome;
        private final DetailViewer m_phonebookViewerOffice;

        public Details(SearchConstants constants) {
            m_phonebookViewerGeneral = new PhonebookViewer(constants, FIELDS_GENERAL, FIELDS_PHONENUMBERS);
            m_phonebookViewerHome = new PhonebookViewer(constants, FIELDS_HOME, FIELDS_PHONENUMBERS);
            m_phonebookViewerOffice = new PhonebookViewer(constants, FIELDS_OFFICE, FIELDS_PHONENUMBERS);

            Tab tabGeneral = new Tab(s_searchConstants.tabGeneral());
            Tab tabHome = new Tab(s_searchConstants.tabHome());
            Tab tabOffice = new Tab(s_searchConstants.tabOffice());

            tabGeneral.setPane(m_phonebookViewerGeneral);
            tabHome.setPane(m_phonebookViewerHome);
            tabOffice.setPane(m_phonebookViewerOffice);

            setTabBarPosition(Side.TOP);
            addTab(tabGeneral);
            addTab(tabHome);
            addTab(tabOffice);
        }

        public void setData(ListGridRecord[] selection) {
            m_phonebookViewerGeneral.setData(selection);
            m_phonebookViewerHome.setData(selection);
            m_phonebookViewerOffice.setData(selection);
        }
    }

    private static class PhonebookViewer extends DetailViewer {
        private static final String EMPTY_STRING = "";

        public PhonebookViewer(SearchConstants constants, String[] fieldNames, String[] fieldPhoneNumbers) {
            setEmptyMessage(constants.selectUser());
            setFields(createFields(fieldNames, constants, fieldPhoneNumbers));
        }

        /**
         * This method strips the phone number of non-digit characters, except for + sign.
         *
         * @param phoneNumber the number to format
         * @return the formatted number
         */
        public String formatPhoneNumber(String phoneNumber) {
            return phoneNumber.replaceAll("[^0-9+]", EMPTY_STRING);
        }

        private DetailViewerField[] createFields(String[] fieldNames, ConstantsWithLookup constants,
                String[] fieldsPhoneNumbers) {
            DetailViewerField[] fields = new DetailViewerField[fieldNames.length];
            for (int i = 0; i < fields.length; i++) {
                String title = constants.getString(fieldNames[i]);
                fields[i] = new DetailViewerField(fieldNames[i], title);

                if (Arrays.asList(fieldsPhoneNumbers).contains(fieldNames[i])) {
                    final String altText = constants.getString("clickToCall");
                    fields[i].setDetailFormatter(new DetailFormatter() {
                        @Override
                        public String format(Object value, DetailViewerRecord record, DetailViewerField field) {

                            String img = Img.getImgURL("/sipxconfig/images/phone.png");

                            if (value != null) {
                                return value +  "<a class=\"gwtClickToCall\" onclick=\"clickToCall('"
                                        + formatPhoneNumber((String) value) + "')\"> <img class=\""
                                        + "gwtClickToCallImg\" align=\"right\" src=\"" + img + "\" title=\""
                                        + altText + "\"/> </a>";
                            }
                            return EMPTY_STRING;
                        }
                    });
                }
            }
            return fields;
        }
    }
}
