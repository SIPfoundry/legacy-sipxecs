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
import com.google.gwt.user.client.Timer;
import com.google.gwt.user.client.ui.RootPanel;
import com.smartgwt.client.data.Criteria;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.types.Alignment;
import com.smartgwt.client.types.Overflow;
import com.smartgwt.client.types.Side;
import com.smartgwt.client.types.Visibility;
import com.smartgwt.client.util.BooleanCallback;
import com.smartgwt.client.util.SC;
import com.smartgwt.client.widgets.Canvas;
import com.smartgwt.client.widgets.IButton;
import com.smartgwt.client.widgets.Img;
import com.smartgwt.client.widgets.Label;
import com.smartgwt.client.widgets.Window;
import com.smartgwt.client.widgets.events.ClickEvent;
import com.smartgwt.client.widgets.events.ClickHandler;
import com.smartgwt.client.widgets.events.CloseClickHandler;
import com.smartgwt.client.widgets.events.CloseClientEvent;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.ValuesManager;
import com.smartgwt.client.widgets.form.fields.FormItem;
import com.smartgwt.client.widgets.form.fields.PasswordItem;
import com.smartgwt.client.widgets.form.fields.SelectItem;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.fields.events.ChangedEvent;
import com.smartgwt.client.widgets.form.fields.events.ChangedHandler;
import com.smartgwt.client.widgets.form.validator.RegExpValidator;
import com.smartgwt.client.widgets.form.validator.RequiredIfFunction;
import com.smartgwt.client.widgets.form.validator.RequiredIfValidator;
import com.smartgwt.client.widgets.grid.ListGrid;
import com.smartgwt.client.widgets.grid.ListGridField;
import com.smartgwt.client.widgets.grid.ListGridRecord;
import com.smartgwt.client.widgets.grid.events.RecordClickEvent;
import com.smartgwt.client.widgets.grid.events.RecordClickHandler;
import com.smartgwt.client.widgets.layout.HLayout;
import com.smartgwt.client.widgets.layout.VLayout;
import com.smartgwt.client.widgets.tab.Tab;
import com.smartgwt.client.widgets.tab.TabSet;

import org.sipfoundry.sipxconfig.userportal.widget.PagedPhonebookDataSource;
import org.sipfoundry.sipxconfig.userportal.widget.PhonebookDataSource;

// FIXME: all sizes should be in CSS
public class UserPhonebookSearch implements EntryPoint {

    private static final String DUMMY_ID = "-1";
    private static final String EMPTY_STRING = "";
    private static final String EMAIL_EXPRESSION = "^([a-zA-Z0-9_.\\-+])+@(([a-zA-Z0-9\\-])+\\.)+[a-zA-Z0-9]{2,4}$";
    private static final String WILD_CARD = "*";

    private static final String[] FIELDS_PHONENUMBERS = {
        PhonebookDataSource.NUMBER, PhonebookDataSource.CELL_PHONE_NUMBER, PhonebookDataSource.HOME_PHONE_NUMBER,
        PhonebookDataSource.ASSISTANT_PHONE_NUMBER
    };

    private static SearchConstants s_searchConstants = GWT.create(SearchConstants.class);

    @Override
    public void onModuleLoad() {
        final SizeLabel sizeLabel = new SizeLabel();
        sizeLabel.setWidth100();

        final DataSource phonebookDS = new PagedPhonebookDataSource("phonebookGridId") {
            @Override
            protected void onDataFetch(Integer totalRows, Integer filteredRows) {
                sizeLabel.update(totalRows, filteredRows);
            }
        };

        final PhonebookGrid phonebookGrid = new PhonebookGrid(phonebookDS);
        final Details details = new Details(phonebookGrid);
        details.addData();

        final Details detailsLayout = new Details(phonebookGrid, 6, 6, 6);
        detailsLayout.addData();

        phonebookGrid.addRecordClickHandler(new RecordClickHandler() {
            @Override
            public void onRecordClick(RecordClickEvent event) {
                String number = event.getRecord().getAttributeAsString(PhonebookDataSource.NUMBER);
                RootPanel.get("call:number").getElement().setPropertyString("value", number);
                int recordIndex = event.getRecordNum();
                phonebookGrid.collapeAllRecordsAndExpandThisOne(phonebookGrid.getRecord(recordIndex), recordIndex);
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

        TabSet tabSet = new TabSet();
        tabSet.setTabBarPosition(Side.TOP);
        tabSet.setWidth("75%");
        tabSet.setHeight(490);

        Tab entriesTab = new Tab(s_searchConstants.entries());

        HLayout leftHLayout = new HLayout();
        leftHLayout.addMember(searchForm);
        leftHLayout.addMember(sizeLabel);
        leftHLayout.setWidth("50%");

        HLayout rightHLayout = new HLayout();
        rightHLayout.addMember(gmailImport);
        rightHLayout.setWidth(WILD_CARD);
        rightHLayout.setAlign(Alignment.RIGHT);

        HLayout topHLayout = new HLayout();
        topHLayout.addMember(leftHLayout);
        topHLayout.addMember(rightHLayout);
        topHLayout.setWidth100();
        topHLayout.setHeight(30);

        VLayout layout = new VLayout(5);
        layout.setMargin(5);
        layout.addMember(topHLayout);
        layout.addMember(phonebookGrid);
        entriesTab.setPane(layout);

        Tab newContactTab = new Tab(s_searchConstants.addNewContact());
        newContactTab.setPane(new DetailsLayout(detailsLayout, phonebookGrid));

        tabSet.addTab(entriesTab);
        tabSet.addTab(newContactTab);

        RootPanel.get("user_phonebook_grid").add(tabSet);

    }

    private static class PhonebookGrid extends ListGrid {
        private static final String NRML_TABLE_FIELD_WIDTH = "20%";
        private ListGridRecord m_expandedRecord;
        private int m_expandedRecordIndex = Integer.valueOf(DUMMY_ID);

        public PhonebookGrid(DataSource phonebookDS) {
            ListGridField firstNameField = new ListGridField(PhonebookDataSource.FIRST_NAME, s_searchConstants
                    .firstName());
            ListGridField lastNameField = new ListGridField(PhonebookDataSource.LAST_NAME, s_searchConstants
                    .lastName());
            ListGridField numberField = new ListGridField(PhonebookDataSource.NUMBER, s_searchConstants.number());
            ListGridField emailField = new ListGridField(PhonebookDataSource.EMAIL_ADDRESS, s_searchConstants
                    .emailAddress());

            firstNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            lastNameField.setWidth(NRML_TABLE_FIELD_WIDTH);
            numberField.setWidth(NRML_TABLE_FIELD_WIDTH);
            emailField.setWidth(WILD_CARD);

            setEmptyMessage(s_searchConstants.noUserFound());
            setWidth100();
            setHeight100();

            setBodyOverflow(Overflow.AUTO);
            setOverflow(Overflow.VISIBLE);
            setAlternateRecordStyles(true);
            setDataSource(phonebookDS);
            setAutoFetchData(true);
            setDataPageSize(100);

            setCellHeight(25);
            setHeaderHeight(35);
            setFields(firstNameField, lastNameField, numberField, emailField);

            setCanExpandRecords(true);
        }

        public void collapeAllRecordsAndExpandThisOne(ListGridRecord record, int recordIndex) {
            if (null != m_expandedRecord) {
                collapseRecord(m_expandedRecord);
            }

            if (m_expandedRecordIndex != recordIndex) {
                expandRecord(record);
                m_expandedRecord = record;
                m_expandedRecordIndex = recordIndex;
            } else {
                m_expandedRecord = null;
                m_expandedRecordIndex = Integer.valueOf(DUMMY_ID);
            }
            this.scrollToRow(recordIndex);
        }

        public void refreshRecordsWithDelay() {
            setOpacity(50);
            // fetch data with 1.5 secs delay so the changes to be picked up
            Timer refreshTimer = new Timer() {
                @Override
                public void run() {
                    setRecords(new ListGridRecord[0]);
                    fetchData();
                    setOpacity(null);
                }
            };
            refreshTimer.schedule(1500);
        }

        @Override
        protected Canvas getExpansionComponent(final ListGridRecord record) {
            return new ExpandedContactComponent(this);
        }
    }

    private static class SearchForm extends DynamicForm {
        private final TextItem m_query;

        public SearchForm(DataSource phonebookDS) {
            m_query = new TextItem();
            m_query.setTitle(s_searchConstants.searchWidgetTitle());

            setFields(m_query);
        }

        void addHandler(final ListGrid phonebookGrid) {
            ChangedHandler onChange = new ChangedHandler() {
                @Override
                public void onChanged(ChangedEvent event) {
                    phonebookGrid.invalidateCache();
                    if (m_query.getValue() != null) {
                        phonebookGrid
                                .filterData(new Criteria(PhonebookDataSource.QUERY, (String) m_query.getValue()));
                    } else {
                        phonebookGrid.filterData();
                    }
                }
            };
            m_query.addChangedHandler(onChange);
        }
    }


    private static final class ExpandedContactComponent extends VLayout {
        private final IButton m_editButton = new IButton(s_searchConstants.edit());
        private final IButton m_saveButton = new IButton(s_searchConstants.save());
        private final IButton m_resetButton = new IButton(s_searchConstants.reset());
        private final IButton m_cancelButton = new IButton(s_searchConstants.cancel());

        public ExpandedContactComponent(final PhonebookGrid grid) {
            setStyleName("gwtExpandedContact");
            setWidth("670px");
            setAutoHeight();

            final ListGridRecord record = grid.getSelectedRecord();

            final Details details = new Details(grid);
            details.setData(record);

            final String entryId = record.getAttributeAsString(PhonebookDataSource.ENTRY_ID);

            IButton removeButton = new IButton(s_searchConstants.delete());
            removeButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    SC.ask(s_searchConstants.confirmDelete(), new BooleanCallback() {
                        @Override
                        public void execute(Boolean value) {
                            if (value != null && value) {
                                ((PhonebookDataSource) grid.getDataSource()).deleteEntry(entryId);
                                grid.refreshRecordsWithDelay();
                            }
                        }
                    });
                }
            });

            hideEditControls();

            m_editButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    details.editData();
                    showEditControls();
                }
            });

            m_saveButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    details.onSaveClick(entryId);
                }
            });

            m_resetButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    details.onResetClick();
                }
            });

            m_cancelButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    details.onCancelClick();
                    hideEditControls();
                }
            });

            HLayout formActions = new HLayout();
            formActions.setMembersMargin(4);
            formActions.setAlign(Alignment.RIGHT);
            formActions.setPadding(10);

            if (!entryId.equals(DUMMY_ID)) {
                formActions.addMember(m_saveButton);
                formActions.addMember(m_resetButton);
                formActions.addMember(m_cancelButton);

                formActions.addMember(m_editButton);
                formActions.addMember(removeButton);
            }

            addMember(new DetailsTab(details));
            addMember(formActions);

        }

        private final void showEditControls() {
            m_saveButton.setVisibility(Visibility.INHERIT);
            m_resetButton.setVisibility(Visibility.INHERIT);
            m_cancelButton.setVisibility(Visibility.INHERIT);
            m_editButton.setVisibility(Visibility.HIDDEN);
        }

        private final void hideEditControls() {
            m_saveButton.setVisibility(Visibility.HIDDEN);
            m_resetButton.setVisibility(Visibility.HIDDEN);
            m_cancelButton.setVisibility(Visibility.HIDDEN);
            m_editButton.setVisibility(Visibility.INHERIT);
        }
    }



    private static final class DetailsTab extends TabSet {
        public DetailsTab(Details details) {
            setHeight("260px");

            HLayout generalLayout = new HLayout();
            generalLayout.addMember(details.getAvatarSection());
            generalLayout.addMember(details.getGeneralForm());

            HLayout addressLayout = new HLayout();
            addressLayout.addMember(details.getHomeForm());
            addressLayout.addMember(details.getOfficeForm());
            addressLayout.setMembersMargin(10);
            addressLayout.setWidth100();

            Tab generalTab = new Tab();
            generalTab.setPane(generalLayout);
            generalTab.setTitle(s_searchConstants.generalForm());

            Tab homeTab = new Tab();
            homeTab.setPane(addressLayout);
            homeTab.setTitle(s_searchConstants.address());

            addTab(generalTab);
            addTab(homeTab);
        }
    }



    private static final class DetailsLayout extends VLayout {
        public DetailsLayout(final Details details, final PhonebookGrid grid) {
            final IButton saveButton = new IButton(s_searchConstants.save());
            saveButton.addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    details.onSaveClick(DUMMY_ID);
                }
            });

            Label title = new Label(s_searchConstants.newContact());
            title.setStyleName("gwtHeading");
            title.setAutoHeight();
            title.setWidth100();
            title.setAlign(Alignment.CENTER);

            addMember(title);
            addMember(details.getGeneralForm());
            addMember(details.getHomeForm());
            addMember(details.getOfficeForm());
            addMember(saveButton);

            setMembersMargin(10);
        }
    }



    private static class Details {
        private static final String[] FIELDS_GENERAL = {
            PhonebookDataSource.FIRST_NAME, PhonebookDataSource.NUMBER, PhonebookDataSource.EMAIL_ADDRESS,
            PhonebookDataSource.LAST_NAME, PhonebookDataSource.LOCATION,
            PhonebookDataSource.ALTERNATE_EMAIL_ADDRESS, PhonebookDataSource.JOB_TITLE,
            PhonebookDataSource.CELL_PHONE_NUMBER, PhonebookDataSource.IM_ID, PhonebookDataSource.JOB_DEPT,
            PhonebookDataSource.FAX_NUMBER, PhonebookDataSource.ALTERNATE_IM_ID, PhonebookDataSource.COMPANY_NAME
        };

        private static final String[] FIELDS_HOME = {
            PhonebookDataSource.HOME_STREET, PhonebookDataSource.HOME_STATE, PhonebookDataSource.HOME_ZIP,
            PhonebookDataSource.HOME_CITY, PhonebookDataSource.HOME_COUNTRY, PhonebookDataSource.HOME_PHONE_NUMBER
        };

        private static final String[] FIELDS_OFFICE = {
            PhonebookDataSource.OFFICE_DESIGNATION, PhonebookDataSource.OFFICE_STREET,
            PhonebookDataSource.OFFICE_COUNTRY, PhonebookDataSource.ASSISTANT_NAME, PhonebookDataSource.OFFICE_CITY,
            PhonebookDataSource.OFFICE_ZIP, PhonebookDataSource.ASSISTANT_PHONE_NUMBER,
            PhonebookDataSource.OFFICE_STATE
        };

        private static PhonebookGrid s_phonebookGrid;

        private final PhonebookForm m_generalForm;
        private final PhonebookForm m_homeForm;
        private final PhonebookForm m_officeForm;
        private final AvatarSection m_avatar = new AvatarSection();
        private final ValuesManager m_vm;

        private boolean m_edit;

        public Details(final PhonebookGrid grid) {
            this(grid, 4, 2, 2);
        }

        public Details(final PhonebookGrid grid, int generalCols, int homeCols, int officeCols) {
            m_generalForm = new PhonebookForm(FIELDS_GENERAL, "generalForm", generalCols);
            m_homeForm = new PhonebookForm(FIELDS_HOME, "homeForm", homeCols);
            m_officeForm = new PhonebookForm(FIELDS_OFFICE, "officeForm", officeCols);

            m_generalForm.addRequiredValidator(PhonebookDataSource.FIRST_NAME);
            m_generalForm.addRequiredValidator(PhonebookDataSource.LAST_NAME);
            m_generalForm.addRequiredValidator(PhonebookDataSource.NUMBER);
            m_generalForm.addEmailValidator(PhonebookDataSource.EMAIL_ADDRESS);
            m_generalForm.addEmailValidator(PhonebookDataSource.ALTERNATE_EMAIL_ADDRESS);

            m_vm = new ValuesManager();

            m_generalForm.setValuesManager(m_vm);
            m_homeForm.setValuesManager(m_vm);
            m_officeForm.setValuesManager(m_vm);

            s_phonebookGrid = grid;
        }

        public void onSaveClick(String entryId) {
            if (m_edit && (entryId.isEmpty() || entryId.equals(DUMMY_ID))) {
                SC.say(s_searchConstants.editRecordWarning());
            } else {
                if (m_vm.validate() && m_vm.valuesHaveChanged()) {
                    if (m_edit) {
                        ((PhonebookDataSource) s_phonebookGrid.getDataSource()).editEntry(entryId, m_vm);
                        s_phonebookGrid.refreshRecordsWithDelay();
                    } else {
                        ((PhonebookDataSource) s_phonebookGrid.getDataSource()).addEntry(entryId, m_vm);
                        s_phonebookGrid.refreshRecordsWithDelay();
                        addData();
                    }
                }
            }
        }

        public void onResetClick() {
            m_vm.resetValues();
        }

        public void onCancelClick() {
            m_vm.clearErrors(true);
            setData(s_phonebookGrid.getSelectedRecord());
        }

        public void setData(ListGridRecord selection) {
            m_avatar.update(selection);
            m_generalForm.setData(selection);
            m_homeForm.setData(selection);
            m_officeForm.setData(selection);
        }

        public void editData() {
            m_vm.rememberValues();
            m_edit = true;

            m_generalForm.editData();
            m_homeForm.editData();
            m_officeForm.editData();
        }

        public void addData() {
            m_vm.clearValues();
            m_edit = false;

            m_generalForm.addData();
            m_homeForm.addData();
            m_officeForm.addData();
        }

        public AvatarSection getAvatarSection() {
            return m_avatar;
        }

        public PhonebookForm getGeneralForm() {
            return m_generalForm;
        }

        public PhonebookForm getHomeForm() {
            return m_homeForm;
        }

        public PhonebookForm getOfficeForm() {
            return m_officeForm;
        }
    }



    private static final class PhonebookForm extends DynamicForm {
        private static final String TITLE_STYLE = "titleFormStyle";

        public PhonebookForm(String[] fieldNames, String formName, int numCols) {
            setGroupTitle(s_searchConstants.getString(formName));
            setIsGroup(true);
            setNumCols(numCols);
            setFixedColWidths(true);
            setFields(createFields(fieldNames, s_searchConstants));
        }

        public void setData(ListGridRecord selection) {
            for (FormItem item : getFields()) {
                String itemName = item.getName();
                String selectionValue = selection.getAttributeAsString(itemName);
                item.setValue(selectionValue);
                item.setDisabled(true);
                item.setTitleStyle(TITLE_STYLE);
            }
        }

        public void editData() {
            for (FormItem item : getFields()) {
                item.setDisabled(false);
            }
        }

        public void addData() {
            for (FormItem item : getFields()) {
                item.setDisabled(false);
                item.setValue(EMPTY_STRING);
            }
        }

        private TextItem[] createFields(String[] fieldNames, ConstantsWithLookup constants) {
            TextItem[] fields = new TextItem[fieldNames.length];
            for (int i = 0; i < fields.length; i++) {
                String title = constants.getString(fieldNames[i]);
                fields[i] = new TextItem(fieldNames[i], title);
                fields[i].setTitleStyle(TITLE_STYLE);
            }
            return fields;
        }

        public void addRequiredValidator(String name) {
            FormItem item = getItem(name);
            RequiredIfValidator requiredValidator = new RequiredIfValidator();
            requiredValidator.setExpression(new RequiredIfFunction() {

                @Override
                public boolean execute(FormItem formItem, Object value) {
                    return true;
                }
            });
            requiredValidator.setErrorMessage(s_searchConstants.requiredField());
            item.setValidators(requiredValidator);
        }

        public void addEmailValidator(String name) {
            FormItem item = getItem(name);
            RegExpValidator emailValidator = new RegExpValidator();
            emailValidator.setErrorMessage(s_searchConstants.invalidEmail());
            emailValidator.setExpression(EMAIL_EXPRESSION);
            item.setValidators(emailValidator);
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
        return phoneNumber.replaceAll("[^0-9+]", EMPTY_STRING);
    }

    private static class SizeLabel extends Label {
        public void update(Integer totalRows, Integer filteredRows) {
            setContents(s_searchConstants.numberOfEntries() + EMPTY_STRING + filteredRows + "/" + totalRows);
        }
    }

    private static class AvatarSection extends VLayout {
        private static final String DEFAULT_AVATAR_URL = "http://gravatar.com/avatar";

        private final Img m_avatar;
        private final Label m_contactName;
        private final ClickToCallButton m_clickToCall;

        public AvatarSection() {
            m_avatar = new Img();
            m_avatar.setStyleName("gwtAvatarImg");
            m_avatar.setWidth(140);
            m_avatar.setHeight(140);
            m_avatar.setLayoutAlign(Alignment.CENTER);

            m_contactName = new Label();
            m_contactName.setStyleName("gwtContactNameStyle");
            m_contactName.setAlign(Alignment.CENTER);
            m_contactName.setAutoHeight();

            final HLayout clickToCallButtonLayout = new HLayout();
            m_clickToCall = new ClickToCallButton();
            clickToCallButtonLayout.addMember(m_clickToCall);
            clickToCallButtonLayout.setPadding(15);
            clickToCallButtonLayout.setWidth100();
            clickToCallButtonLayout.setAlign(Alignment.CENTER);

            addMember(m_avatar);
            addMember(m_contactName);
            addMember(clickToCallButtonLayout);

            setWidth(150);
            setHeight100();
        }

        public void update(ListGridRecord record) {
            String avatar = getNullSafeValue(record.getAttributeAsString(PhonebookDataSource.AVATAR));
            if (avatar.isEmpty()) {
                avatar = DEFAULT_AVATAR_URL;
            }

            String firstName = getNullSafeValue(record.getAttributeAsString(PhonebookDataSource.FIRST_NAME));
            String lastName = getNullSafeValue(record.getAttributeAsString(PhonebookDataSource.LAST_NAME));
            m_clickToCall.update(record);
            refreshSection(avatar, firstName + " " + lastName);
        }

        private void refreshSection(String imgSrc, String contactName) {
            m_avatar.setSrc(imgSrc);
            m_contactName.setContents(contactName);
        }

        private String getNullSafeValue(String value) {
            if (value == null || value.equalsIgnoreCase("null")) {
                return EMPTY_STRING;
            }
            return value;
        }
    }



    private static class ClickToCallButton extends IButton {
        private final LinkedHashMap<String, String> m_phoneMap;

        public ClickToCallButton() {
            super(s_searchConstants.call());
            m_phoneMap = new LinkedHashMap<String, String>();

            addClickHandler(new ClickHandler() {
                @Override
                public void onClick(ClickEvent event) {
                    if (!m_phoneMap.isEmpty()) {
                        final ClickToCallModalWindow winModal = new ClickToCallModalWindow(m_phoneMap);
                        winModal.show();
                    }
                }
            });
        }

        public void update(ListGridRecord listGridRecord) {
            m_phoneMap.clear();

            if (null == listGridRecord) {
                return;
            }

            for (String fieldName : FIELDS_PHONENUMBERS) {
                String numberToDial = listGridRecord.getAttributeAsString(fieldName);
                if (numberToDial != null) {
                    m_phoneMap.put(fieldName + numberToDial, s_searchConstants.getString(fieldName));
                }
            }
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
            importFromGmail.setAutoFit(true);
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
