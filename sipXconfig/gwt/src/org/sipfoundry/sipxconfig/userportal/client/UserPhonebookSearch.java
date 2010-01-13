/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import java.util.LinkedHashMap;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.http.client.Request;
import com.google.gwt.http.client.RequestBuilder;
import com.google.gwt.http.client.RequestCallback;
import com.google.gwt.http.client.RequestException;
import com.google.gwt.http.client.Response;
import com.google.gwt.i18n.client.ConstantsWithLookup;
import com.google.gwt.user.client.ui.RootPanel;
import com.smartgwt.client.data.Criteria;
import com.smartgwt.client.data.DSRequest;
import com.smartgwt.client.data.DSResponse;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.data.XMLTools;
import com.smartgwt.client.types.Alignment;
import com.smartgwt.client.types.DSOperationType;
import com.smartgwt.client.types.Overflow;
import com.smartgwt.client.types.Side;
import com.smartgwt.client.util.SC;
import com.smartgwt.client.widgets.IButton;
import com.smartgwt.client.widgets.Label;
import com.smartgwt.client.widgets.Window;
import com.smartgwt.client.widgets.events.ClickEvent;
import com.smartgwt.client.widgets.events.ClickHandler;
import com.smartgwt.client.widgets.events.CloseClickHandler;
import com.smartgwt.client.widgets.events.CloseClientEvent;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.fields.FormItem;
import com.smartgwt.client.widgets.form.fields.PasswordItem;
import com.smartgwt.client.widgets.form.fields.SelectItem;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.fields.events.ChangedEvent;
import com.smartgwt.client.widgets.form.fields.events.ChangedHandler;
import com.smartgwt.client.widgets.grid.HoverCustomizer;
import com.smartgwt.client.widgets.grid.ListGrid;
import com.smartgwt.client.widgets.grid.ListGridField;
import com.smartgwt.client.widgets.grid.ListGridRecord;
import com.smartgwt.client.widgets.grid.events.RecordClickEvent;
import com.smartgwt.client.widgets.grid.events.RecordClickHandler;
import com.smartgwt.client.widgets.grid.events.RecordDoubleClickEvent;
import com.smartgwt.client.widgets.grid.events.RecordDoubleClickHandler;
import com.smartgwt.client.widgets.layout.HLayout;
import com.smartgwt.client.widgets.layout.VLayout;
import com.smartgwt.client.widgets.tab.Tab;
import com.smartgwt.client.widgets.tab.TabSet;
import com.smartgwt.client.widgets.viewer.DetailViewer;
import com.smartgwt.client.widgets.viewer.DetailViewerField;

import org.sipfoundry.sipxconfig.userportal.widget.PhonebookDataSource;

// FIXME: all sizes should be in CSS
public class UserPhonebookSearch implements EntryPoint {
    private static final String FIRST_NAME = "first-name";
    private static final String LAST_NAME = "last-name";
    private static final String NUMBER = "number";
    private static final String EMAIL = "emailAddress";
    private static final String QUERY = "query";

    private static final String PHONE_CELL = "cellPhoneNumber";
    private static final String PHONE_HOME = "homePhoneNumber";
    private static final String PHONE_ASST = "assistantPhoneNumber";

    private static final String WILD_CARD = "*";

    private static final String[] FIELDS_PHONENUMBERS = {
        NUMBER, PHONE_CELL, PHONE_HOME, PHONE_ASST
    };

    private static SearchConstants s_searchConstants = GWT.create(SearchConstants.class);

    @Override
    public void onModuleLoad() {
        final SizeLabel sizeLabel = new SizeLabel();

        DataSource phonebookDS = new PhonebookDataSource("phonebookGridId") {
            @Override
            protected Object transformRequest(DSRequest request) {

                if (request.getOperationType().equals(DSOperationType.FETCH)) {
                    request.setActionURL(getDataURL() + "?start=" + request.getStartRow() + "&end="
                            + request.getEndRow() + "&filter=" + request.getCriteria().getAttributeAsString(QUERY));
                }

                return super.transformRequest(request);
            }

            @Override
            protected void transformResponse(DSResponse response, DSRequest request, Object data) {

                if (request.getOperationType().equals(DSOperationType.FETCH)) {
                    Integer size = Integer.parseInt(XMLTools.selectString(data, "/phonebook/size"));
                    Integer filteredSize = Integer.parseInt(XMLTools.selectString(data, "/phonebook/filtered-size"));
                    Integer startRow = Integer.parseInt(XMLTools.selectString(data, "/phonebook/start-row"));
                    Integer endRow = Integer.parseInt(XMLTools.selectString(data, "/phonebook/end-row"));

                    response.setTotalRows(filteredSize);
                    response.setStartRow(startRow);
                    response.setEndRow(endRow);
                    sizeLabel.update(size, filteredSize);
                } else {
                    super.transformResponse(response, request, data);
                }

            }
        };

        final Details topTabSet = new Details(s_searchConstants);

        final ListGrid phonebookGrid = new PhonebookGrid(phonebookDS);
        phonebookGrid.addRecordClickHandler(new RecordClickHandler() {
            @Override
            public void onRecordClick(RecordClickEvent event) {
                topTabSet.setData(phonebookGrid.getSelection());
                String number = event.getRecord().getAttributeAsString(NUMBER);
                RootPanel.get("call:number").getElement().setPropertyString("value", number);
            }
        });

        SearchForm searchForm = new SearchForm(phonebookDS);
        searchForm.addHandler(phonebookGrid);

        IButton gmailImport = new IButton(s_searchConstants.gmailImportTitle());
        gmailImport.addClickHandler(new ClickHandler() {
            @Override
            public void onClick(ClickEvent event) {
                final GmailImportModalWindow gmailModal = new GmailImportModalWindow(phonebookGrid);
                gmailModal.show();
            }
        });
        gmailImport.setAutoFit(true);

        HLayout leftHLayout = new HLayout();
        leftHLayout.addMember(searchForm);
        leftHLayout.setWidth("50%");

        HLayout rightHLayout = new HLayout();
        rightHLayout.addMember(gmailImport);
        rightHLayout.setWidth(WILD_CARD);
        rightHLayout.setAlign(Alignment.RIGHT);

        HLayout topHLayout = new HLayout();
        topHLayout.addMember(leftHLayout);
        topHLayout.addMember(rightHLayout);
        topHLayout.setWidth100();

        HLayout hgridLayout = new HLayout();
        hgridLayout.setLayoutRightMargin(5);
        hgridLayout.addMember(phonebookGrid);
        hgridLayout.addMember(topTabSet);
        hgridLayout.setWidth("100%");
        hgridLayout.setHeight(300);

        VLayout gridLayout = new VLayout();
        gridLayout.setLayoutBottomMargin(5);
        gridLayout.addMember(topHLayout);
        gridLayout.addMember(sizeLabel);
        gridLayout.addMember(hgridLayout);
        gridLayout.setWidth("70%");

        RootPanel.get("user_phonebook_grid").add(gridLayout);
    }

    private static class PhonebookGrid extends ListGrid {
        private static final String NRML_TABLE_FIELD_WIDTH = "20%";

        public PhonebookGrid(DataSource phonebookDS) {
            ListGridField firstNameField = new ListGridField(FIRST_NAME, s_searchConstants.firstName());
            ListGridField lastNameField = new ListGridField(LAST_NAME, s_searchConstants.lastName());
            ListGridField numberField = new ListGridField(NUMBER, s_searchConstants.number());
            ListGridField emailField = new ListGridField(EMAIL, s_searchConstants.emailAddress());

            firstNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            lastNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            numberField.setWidth(NRML_TABLE_FIELD_WIDTH);
            emailField.setWidth(WILD_CARD);

            setHeaderTitleStyle("gwtGridHeaderTitle");
            setBaseStyle("gwtGridBody");
            setEmptyMessage(s_searchConstants.noUserFound());
            setWidth("71%");

            setBodyOverflow(Overflow.AUTO);
            setOverflow(Overflow.VISIBLE);
            setAlternateRecordStyles(true);
            setDataSource(phonebookDS);
            setAutoFetchData(true);
            setDataPageSize(100);

            setCellHeight(25);
            setHeaderHeight(35);
            setFields(firstNameField, lastNameField, numberField, emailField);
            setupClickToCallRowHover();

            addRecordDoubleClickHandler(new RecordDoubleClickHandler() {
                public void onRecordDoubleClick(RecordDoubleClickEvent event) {
                    final LinkedHashMap<String, String> map = new LinkedHashMap<String, String>();

                    for (String fieldName : FIELDS_PHONENUMBERS) {
                        String numberToDial = event.getRecord().getAttributeAsString(fieldName);
                        if (numberToDial != null) {
                            map.put(fieldName + numberToDial, s_searchConstants.getString(fieldName));
                        }
                    }

                    final ClickToCallModalWindow winModal = new ClickToCallModalWindow(map);
                    winModal.show();
                }
            });
        }

        // Set ClickToCall hover prompt to all the fields in a row
        private void setupClickToCallRowHover() {
            ListGridField[] listGridFields = getFields();

            for (ListGridField listGridField : listGridFields) {
                listGridField.setShowHover(true);
                listGridField.setHoverCustomizer(new HoverCustomizer() {
                    @Override
                    public String hoverHTML(Object value, ListGridRecord record, int rowNum, int colNum) {
                        return s_searchConstants.clickToDialHoverInfo();
                    }
                });
            }
        }
    }

    private static class SearchForm extends DynamicForm {
        private final TextItem m_query;

        public SearchForm(DataSource phonebookDS) {
            m_query = new TextItem();
            m_query.setTitle(s_searchConstants.searchWidgetTitle());

            setFields(m_query);
            setDataSource(phonebookDS);
        }

        void addHandler(final ListGrid phonebookGrid) {
            ChangedHandler onChange = new ChangedHandler() {
                @Override
                public void onChanged(ChangedEvent event) {
                    if (m_query.getValue() != null) {
                        phonebookGrid.filterData(new Criteria(QUERY, (String) m_query.getValue()));
                    } else {
                        phonebookGrid.filterData();
                    }
                }
            };
            m_query.addChangedHandler(onChange);
        }
    }

    private static class Details extends TabSet {

        private static final String[] FIELDS_GENERAL = {
            "jobTitle", "jobDept", "companyName", "location", PHONE_CELL, "faxNumber", "imId", "alternateImId",
            "alternateEmailAddress"
        };

        private static final String[] FIELDS_HOME = {
            "homeStreet", "homeCity", "homeState", "homeCountry", "homeZip", PHONE_HOME
        };

        private static final String[] FIELDS_OFFICE = {
            "officeDesignation", "assistantName", PHONE_ASST, "officeStreet", "officeCity", "officeState",
            "officeCountry", "officeZip"
        };

        private final DetailViewer m_phonebookViewerGeneral;
        private final DetailViewer m_phonebookViewerHome;
        private final DetailViewer m_phonebookViewerOffice;

        public Details(SearchConstants constants) {
            m_phonebookViewerGeneral = new PhonebookViewer(constants, FIELDS_GENERAL);
            m_phonebookViewerHome = new PhonebookViewer(constants, FIELDS_HOME);
            m_phonebookViewerOffice = new PhonebookViewer(constants, FIELDS_OFFICE);

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

        public PhonebookViewer(SearchConstants constants, String[] fieldNames) {
            setEmptyMessage(constants.selectUser());
            setFields(createFields(fieldNames, constants));
        }

        private DetailViewerField[] createFields(String[] fieldNames, ConstantsWithLookup constants) {
            DetailViewerField[] fields = new DetailViewerField[fieldNames.length];
            for (int i = 0; i < fields.length; i++) {
                String title = constants.getString(fieldNames[i]);
                fields[i] = new DetailViewerField(fieldNames[i], title);
            }
            return fields;
        }
    }

    private static class ModalWindow extends Window {

        private void createWindow(String title, String help, DynamicForm form, IButton submitButton) {
            setAutoSize(true);

            setTitle(title);
            setShowMinimizeButton(false);
            setIsModal(true);
            setShowModalMask(true);
            centerInPage();
            addCloseClickHandler(new CloseClickHandler() {
                public void onCloseClick(CloseClientEvent event) {
                    destroy();
                }
            });

            Label helpText = new Label();
            helpText.setContents(help);
            helpText.setWrap(false);
            helpText.setStyleName("formHint");
            helpText.setAutoFit(true);

            VLayout importLayout = new VLayout();
            importLayout.addMember(helpText);
            importLayout.addMember(form);
            importLayout.addMember(submitButton);
            importLayout.setAutoHeight();
            importLayout.setAutoWidth();
            importLayout.setLayoutAlign(Alignment.CENTER);
            addItem(importLayout);

        }

    }

    private static class ClickToCallModalWindow extends ModalWindow {

        public ClickToCallModalWindow(LinkedHashMap<String, String> phoneMap) {

            final TextItem numberToDail = new TextItem();
            numberToDail.setShowTitle(false);
            if (0 < phoneMap.keySet().size()) {
                numberToDail.setValue(formatPhoneNumber((String) phoneMap.keySet().toArray()[0]));
            }

            final SelectItem selectNumber = new SelectItem();
            selectNumber.setValueMap(phoneMap);
            selectNumber.addChangedHandler(new ChangedHandler() {
                @Override
                public void onChanged(ChangedEvent event) {
                    String number = (String) event.getValue();
                    numberToDail.setValue(formatPhoneNumber(number));
                }
            });
            selectNumber.setShowTitle(false);
            selectNumber.setDefaultToFirstOption(true);

            final DynamicForm form = new DynamicForm();
            form.setFields(new FormItem[] {
                numberToDail, selectNumber
            });
            form.setLayoutAlign(Alignment.CENTER);

            IButton callButton = new IButton();
            callButton.setTitle(s_searchConstants.call());
            callButton.setLayoutAlign(Alignment.CENTER);
            callButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    if (!((String) numberToDail.getValue()).isEmpty()) {
                        clickToCallRestCall((String) numberToDail.getValue());
                        destroy();
                    }
                }
            });

            super.createWindow(s_searchConstants.clickToCall(), s_searchConstants.clickToCallHelp(), form,
                    callButton);

        }
    }

    private static void clickToCallRestCall(String number) {
        HttpRequestBuilder.doPut("/sipxconfig/rest/my/call/" + number);
    }

    /**
     * This method strips the phone number of non-digit characters, except for + sign.
     *
     * @param phoneNumber the number to format
     * @return the formatted number
     */
    private static String formatPhoneNumber(String phoneNumber) {
        return phoneNumber.replaceAll("[^0-9+]", "");
    }

    private static class SizeLabel extends Label {
        public SizeLabel() {
            setTop(20);
            setHeight(30);
        }

        public void update(Integer totalRows, Integer filteredRows) {
            setContents(s_searchConstants.numberOfEntries() + filteredRows + "/" + totalRows);
        }
    }

    private static class GmailImportModalWindow extends ModalWindow {

        private static final String GMAIL_FIELD_NAME = "mail";
        private static final String PASSWORD_FIELD_NAME = "pwd";
        private static final int WRONG_GMAIL_CREDENTIALS_CODE = 743;
        private static final int INTERNAL_GMAIL_ERROR_CODE = 744;
        private static final int GMAIL_CONNECTION_FAILED_CODE = 745;

        public GmailImportModalWindow(final ListGrid phonebookGrid) {

            final DynamicForm gmailForm = new DynamicForm();
            gmailForm.setNumCols(4);

            TextItem emailItem = new TextItem(GMAIL_FIELD_NAME, s_searchConstants.account());
            PasswordItem passwordItem = new PasswordItem(PASSWORD_FIELD_NAME, s_searchConstants.password());
            gmailForm.setFields(emailItem, passwordItem);

            IButton importFromGmail = new IButton(s_searchConstants.importLabel());
            importFromGmail.setLayoutAlign(Alignment.CENTER);
            importFromGmail.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    String email = gmailForm.getValueAsString(GMAIL_FIELD_NAME);
                    String pass = gmailForm.getValueAsString(PASSWORD_FIELD_NAME);
                    String url = "/sipxconfig/rest/my/phonebook/googleImport?account=" + email + "&password=" + pass;
                    RequestBuilder reqBuilder = new RequestBuilder(RequestBuilder.POST, url);
                    try {
                        reqBuilder.sendRequest(null, new RequestCallback() {

                            @Override
                            public void onResponseReceived(Request request, Response response) {
                                int httpStatusCode = response.getStatusCode();
                                String message = s_searchConstants.gmailImportFailed();
                                if (httpStatusCode == WRONG_GMAIL_CREDENTIALS_CODE) {
                                    message = s_searchConstants.wrongGmailCredentials();
                                } else if (httpStatusCode == INTERNAL_GMAIL_ERROR_CODE) {
                                    message = s_searchConstants.internalGmailProblems();
                                } else if (httpStatusCode == GMAIL_CONNECTION_FAILED_CODE) {
                                    message = s_searchConstants.gmailConnectionFailed();
                                } else if (httpStatusCode == Response.SC_OK) {
                                    message = s_searchConstants.gmailImportSuccess();
                                }
                                SC.say(message);
                                phonebookGrid.setRecords(new ListGridRecord[0]);
                                phonebookGrid.filterData();
                            }

                            @Override
                            public void onError(Request request, Throwable exception) {
                                SC.say(s_searchConstants.gmailImportFailed());
                            }
                        });
                    } catch (RequestException ex) {
                        SC.say(s_searchConstants.gmailImportFailed());
                    }
                    destroy();
                }
            });

            super.createWindow(s_searchConstants.gmailImportTitle(), s_searchConstants.gmailImportHelp(), gmailForm,
                    importFromGmail);
        }

    }
}
