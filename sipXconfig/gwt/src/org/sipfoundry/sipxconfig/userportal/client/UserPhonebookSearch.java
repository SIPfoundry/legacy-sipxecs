/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.ui.RootPanel;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.types.OperatorId;
import com.smartgwt.client.types.VisibilityMode;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.fields.events.ChangedEvent;
import com.smartgwt.client.widgets.form.fields.events.ChangedHandler;
import com.smartgwt.client.widgets.grid.ListGrid;
import com.smartgwt.client.widgets.grid.ListGridField;
import com.smartgwt.client.widgets.grid.events.RecordClickEvent;
import com.smartgwt.client.widgets.grid.events.RecordClickHandler;
import com.smartgwt.client.widgets.layout.SectionStack;
import com.smartgwt.client.widgets.layout.SectionStackSection;
import com.smartgwt.client.widgets.layout.VLayout;
import com.smartgwt.client.widgets.viewer.DetailViewer;
import com.smartgwt.client.widgets.viewer.DetailViewerField;

import org.sipfoundry.sipxconfig.userportal.widget.PhonebookDataSource;

public class UserPhonebookSearch implements EntryPoint {

    private static final String FIRST_NAME = "first-name";
    private static final String LAST_NAME = "last-name";
    private static final String NUMBER = "number";
    private static final String MAX_WIDTH = "100%";

    @Override
    public void onModuleLoad() {

        DataSource phonebookDS = new PhonebookDataSource("phonebookGridId");
        final ListGrid phonebookGrid = new ListGrid();
        phonebookGrid.setEmptyMessage("No users found.");
        phonebookGrid.setWidth(MAX_WIDTH);
        phonebookGrid.setHeight("40%");
        phonebookGrid.setAlternateRecordStyles(true);
        phonebookGrid.setDataSource(phonebookDS);
        phonebookGrid.fetchData();

        ListGridField firstNameField = new ListGridField(FIRST_NAME, "First name");
        ListGridField lastNameField = new ListGridField(LAST_NAME, "Last name");
        ListGridField numberField = new ListGridField(NUMBER, "Phone Number");
        phonebookGrid.setFields(firstNameField, lastNameField, numberField);

        final DynamicForm searchForm = new DynamicForm();
        searchForm.setDataSource(phonebookDS);
        searchForm.setOperator(OperatorId.OR);

        final TextItem searchTextItem = new TextItem();
        searchTextItem.setCriteriaField(FIRST_NAME);
        searchTextItem.setOperator(OperatorId.ISTARTS_WITH);
        searchTextItem.setTitle("Search");
        searchTextItem.setWidth(MAX_WIDTH);

        final TextItem searchTextItem2 = new TextItem();
        searchTextItem2.setCriteriaField(LAST_NAME);
        searchTextItem2.setOperator(OperatorId.ISTARTS_WITH);
        searchTextItem2.setVisible(false);

        final TextItem searchTextItem3 = new TextItem();
        searchTextItem3.setCriteriaField(NUMBER);
        searchTextItem3.setOperator(OperatorId.ISTARTS_WITH);
        searchTextItem3.setVisible(false);

        searchTextItem.addChangedHandler(new ChangedHandler() {
            @Override
            public void onChanged(ChangedEvent event) {
                searchTextItem2.setValue((String) searchTextItem.getValue());
                searchTextItem3.setValue((String) searchTextItem.getValue());
                if (searchTextItem.getValue() != null) {
                    phonebookGrid.filterData(searchForm.getValuesAsCriteria());
                } else {
                    phonebookGrid.filterData();
                }
            }
        });
        searchForm.setFields(searchTextItem, searchTextItem2, searchTextItem3);

        final DetailViewer phonebookViewer = new PhonebookViewer();

        phonebookGrid.addRecordClickHandler(new RecordClickHandler() {
            @Override
            public void onRecordClick(RecordClickEvent event) {
                phonebookViewer.setData(phonebookGrid.getSelection());
            }
        });

        SectionStack contactStack = new SectionStack();
        contactStack.setVisibilityMode(VisibilityMode.MULTIPLE);
        contactStack.setWidth(MAX_WIDTH);
        SectionStackSection contactSection = new SectionStackSection();
        contactSection.setTitle("Contact information");
        contactSection.setExpanded(false);
        contactSection.setItems(phonebookViewer);
        contactStack.setSections(contactSection);

        VLayout gridLayout = new VLayout();
        gridLayout.setWidth(500);
        gridLayout.setHeight(800);
        gridLayout.addMember(searchForm);
        gridLayout.addMember(phonebookGrid);
        gridLayout.addMember(contactStack);

        RootPanel.get("user_phonebook_grid").add(gridLayout);
    }

    private static class PhonebookViewer extends DetailViewer {
        private static final String[][] FIELDS = {
            {
                "jobTitle", "Title"
            }, {
                "jobDept", "Department"
            }, {
                "companyName", "Company"
            }, {
                "assistantName", "Assistant"
            }, {
                "location", "Location"
            }, {
                "cellPhoneNumber", "Cellular number"
            }, {
                "homePhoneNumber", "Home number"
            }, {
                "assistantPhoneNumber", "Assistant number"
            }, {
                "faxNumber", "Fax number"
            }, {
                "imId", "IM id"
            }, {
                "alternateImId", "alternate IM id"
            }, {
                "homeStreet", "Home street"
            }, {
                "homeCity", "Home city"
            }, {
                "homeCountry", "Home country"
            }, {
                "homeState", "Home state"
            }, {
                "homeZip", "Home zip"
            }, {
                "officeStreet", "Office street"
            }, {
                "officeCity", "Office city"
            }, {
                "officeCountry", "Office country"
            }, {
                "officeState", "Office state"
            }, {
                "officeZip", "Office zip"
            }, {
                "officeDesignation", "Office designation"
            },
        };

        public PhonebookViewer() {
            setEmptyMessage("Select a user to see contact information.");
            setHeight(MAX_WIDTH);
            setFields(createFields(FIELDS));
        }

        private DetailViewerField[] createFields(String[][] descriptors) {
            DetailViewerField[] fields = new DetailViewerField[descriptors.length];
            for (int i = 0; i < fields.length; i++) {
                fields[i] = new DetailViewerField(descriptors[i][0], descriptors[i][1]);
            }
            return fields;
        }
    }
}
