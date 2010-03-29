/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.widget;

import com.google.gwt.core.client.GWT;
import com.google.gwt.http.client.Request;
import com.google.gwt.http.client.RequestBuilder;
import com.google.gwt.http.client.RequestCallback;
import com.google.gwt.http.client.RequestException;
import com.google.gwt.http.client.Response;
import com.google.gwt.json.client.JSONObject;
import com.google.gwt.json.client.JSONString;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.data.DataSourceField;
import com.smartgwt.client.data.FieldValueExtractor;
import com.smartgwt.client.data.XMLTools;
import com.smartgwt.client.data.fields.DataSourceTextField;
import com.smartgwt.client.util.SC;
import com.smartgwt.client.widgets.form.ValuesManager;
import org.sipfoundry.sipxconfig.userportal.client.HttpRequestBuilder;
import org.sipfoundry.sipxconfig.userportal.locale.SearchConstants;

public class PhonebookDataSource extends DataSource {

    public static final String ENTRY_ID = "id";
    public static final String FIRST_NAME = "firstName";
    public static final String LAST_NAME = "lastName";
    public static final String NUMBER = "number";
    public static final String FULL_NAME = "fullName";

    public static final String EMAIL_ADDRESS = "emailAddress";
    public static final String CONTACT_INFORMATION = "contact-information";
    public static final String JOB_TITLE = "jobTitle";
    public static final String JOB_DEPT = "jobDept";
    public static final String COMPANY_NAME = "companyName";
    public static final String ASSISTANT_NAME = "assistantName";
    public static final String CELL_PHONE_NUMBER = "cellPhoneNumber";
    public static final String HOME_PHONE_NUMBER = "homePhoneNumber";
    public static final String ASSISTANT_PHONE_NUMBER = "assistantPhoneNumber";
    public static final String FAX_NUMBER = "faxNumber";
    public static final String IM_ID = "imId";
    public static final String IM_DISPLAY_ID = "imDisplayName";
    public static final String ALTERNATE_IM_ID = "alternateImId";
    public static final String ALTERNATE_EMAIL_ADDRESS = "alternateEmailAddress";
    public static final String LOCATION = "location";

    public static final String HOME_ADDRESS = "homeAddress";
    public static final String HOME_STREET = "homeStreet";
    public static final String HOME_CITY = "homeCity";
    public static final String HOME_STATE = "homeState";
    public static final String HOME_COUNTRY = "homeCountry";
    public static final String HOME_ZIP = "homeZip";
    public static final String OFFICE_ADDRESS = "officeAddress";
    public static final String OFFICE_STREET = "officeStreet";
    public static final String OFFICE_CITY = "officeCity";
    public static final String OFFICE_STATE = "officeState";
    public static final String OFFICE_COUNTRY = "officeCountry";
    public static final String OFFICE_ZIP = "officeZip";
    public static final String OFFICE_DESIGNATION = "officeDesignation";
    public static final String AVATAR = "avatar";
    public static final String QUERY = "query";

    private static final String ADDRESS_STREET = "street";
    private static final String ADDRESS_CITY = "city";
    private static final String ADDRESS_STATE = "state";
    private static final String ADDRESS_COUNTRY = "country";
    private static final String ADDRESS_ZIP = "zip";
    private static final String KEY = "entry";
    private static final String VARIANT = "application/json";
    private static final String CONTENT_TYPE = "Content-Type";
    private static final String FIRST_NAME_XPATH = "first-name";
    private static final String LAST_NAME_XPATH = "last-name";

    private static final int  PHONEBOOK_DUPLICATE_ENTRY_ERROR = 747;
    private static SearchConstants s_searchConstants = GWT.create(SearchConstants.class);

    public PhonebookDataSource(String id) {
        super();
        setID(id);

        DataSourceField entryId = new DataSourceTextField(ENTRY_ID);

        DataSourceTextField firstName = new DataSourceTextField(FIRST_NAME);
        firstName.setValueXPath(FIRST_NAME_XPATH);
        DataSourceTextField lastName = new DataSourceTextField(LAST_NAME);
        lastName.setValueXPath(LAST_NAME_XPATH);
        DataSourceTextField number = new DataSourceTextField(NUMBER);

        DataSourceTextField fullName = new DataSourceTextField(FULL_NAME);
        fullName.setFieldValueExtractor(new FieldValueExtractor() {
            @Override
            public Object execute(Object record, Object value, DataSourceField field, String fieldName) {
                String firstName = XMLTools.selectString(record, FIRST_NAME_XPATH);
                String lastName = XMLTools.selectString(record, LAST_NAME_XPATH);
                return getValue(firstName) + " " + getValue(lastName);
            }
        });

        DataSourceField emailAddress = new DataSourceTextField(EMAIL_ADDRESS);
        emailAddress.setValueXPath("contact-information/emailAddress");

        DataSourceField jobTitle = new DataSourceTextField(JOB_TITLE);
        jobTitle.setValueXPath("contact-information/jobTitle");
        DataSourceField jobDept = new DataSourceTextField(JOB_DEPT);
        jobDept.setValueXPath("contact-information/jobDept");
        DataSourceField companyName = new DataSourceTextField(COMPANY_NAME);
        companyName.setValueXPath("contact-information/companyName");
        DataSourceField assistantName = new DataSourceTextField(ASSISTANT_NAME);
        assistantName.setValueXPath("contact-information/assistantName");
        DataSourceField location = new DataSourceTextField(LOCATION);
        location.setValueXPath("contact-information/location");
        DataSourceField cellPhoneNumber = new DataSourceTextField(CELL_PHONE_NUMBER);
        cellPhoneNumber.setValueXPath("contact-information/cellPhoneNumber");
        DataSourceField homePhoneNumber = new DataSourceTextField(HOME_PHONE_NUMBER);
        homePhoneNumber.setValueXPath("contact-information/homePhoneNumber");
        DataSourceField assistantPhoneNumber = new DataSourceTextField(ASSISTANT_PHONE_NUMBER);
        assistantPhoneNumber.setValueXPath("contact-information/assistantPhoneNumber");
        DataSourceField faxNumber = new DataSourceTextField(FAX_NUMBER);
        faxNumber.setValueXPath("contact-information/faxNumber");
        DataSourceField imId = new DataSourceTextField(IM_ID);
        imId.setValueXPath("contact-information/imId");
        DataSourceField alternateImId = new DataSourceTextField(ALTERNATE_IM_ID);
        alternateImId.setValueXPath("contact-information/alternateImId");
        DataSourceField alternateEmailAddress = new DataSourceTextField(ALTERNATE_EMAIL_ADDRESS);
        alternateEmailAddress.setValueXPath("contact-information/alternateEmailAddress");

        DataSourceField homeStreet = new DataSourceTextField(HOME_STREET);
        homeStreet.setValueXPath("contact-information/homeAddress/street");
        DataSourceField homeCity = new DataSourceTextField(HOME_CITY);
        homeCity.setValueXPath("contact-information/homeAddress/city");
        DataSourceField homeCountry = new DataSourceTextField(HOME_COUNTRY);
        homeCountry.setValueXPath("contact-information/homeAddress/country");
        DataSourceField homeState = new DataSourceTextField(HOME_STATE);
        homeState.setValueXPath("contact-information/homeAddress/state");
        DataSourceField homeZip = new DataSourceTextField(HOME_ZIP);
        homeZip.setValueXPath("contact-information/homeAddress/zip");

        DataSourceField officeStreet = new DataSourceTextField(OFFICE_STREET);
        officeStreet.setValueXPath("contact-information/officeAddress/street");
        DataSourceField officeCity = new DataSourceTextField(OFFICE_CITY);
        officeCity.setValueXPath("contact-information/officeAddress/city");
        DataSourceField officeCountry = new DataSourceTextField(OFFICE_COUNTRY);
        officeCountry.setValueXPath("contact-information/officeAddress/country");
        DataSourceField officeState = new DataSourceTextField(OFFICE_STATE);
        officeState.setValueXPath("contact-information/officeAddress/state");
        DataSourceField officeZip = new DataSourceTextField(OFFICE_ZIP);
        officeZip.setValueXPath("contact-information/officeAddress/zip");
        DataSourceField officeDesignation = new DataSourceTextField(OFFICE_DESIGNATION);
        officeDesignation.setValueXPath("contact-information/officeAddress/officeDesignation");

        DataSourceField avatar = new DataSourceTextField(AVATAR);
        avatar.setValueXPath("contact-information/avatar");

        setFields(entryId, firstName, lastName, fullName, number, emailAddress, jobTitle, jobDept, companyName,
                assistantName, location, cellPhoneNumber, homePhoneNumber, assistantPhoneNumber, faxNumber, imId,
                alternateImId, alternateEmailAddress, homeStreet, homeCity, homeCountry, homeState, homeZip,
                officeStreet, officeCity, officeCountry, officeState, officeZip, officeDesignation, avatar);
        setRecordXPath("/phonebook/entry");
        setDataURL("/sipxconfig/rest/my/phonebook");

        /*
         * NOTE - ECLIPSE DEBUGGING
         *
         * REST URL is not accessible when debugging the project from inside eclipse. You can use
         * the data URL "PhonebookTestData.xml" instead. It will give you a set of test data to
         * work with.
         *
         * Make sure to use PhonebookDataSource as the DataSource in UserPhonebookSearch
         *
         * setDataURL("PhonebookTestData.xml");
         */

        setClientOnly(false);
    }

    public void deleteEntry(String entryId, String successMessage) {
        HttpRequestBuilder.doDelete(getPhoneBookEntryUrl(entryId), CONTENT_TYPE, VARIANT, successMessage);
    }

    public void editEntry(String entryId, ValuesManager vm, String successMessage) {
        HttpRequestBuilder.doPut(getPhoneBookEntryUrl(entryId), buildJsonRequest(vm).toString(), CONTENT_TYPE,
                VARIANT, successMessage);
    }

    public void addEntry(String entryId, ValuesManager vm, final String successMessage) {
//        HttpRequestBuilder.doPost(getPhoneBookEntryUrl(entryId), buildJsonRequest(vm).toString(), CONTENT_TYPE,
//                VARIANT, successMessage);
        RequestBuilder reqBuilder = new RequestBuilder(RequestBuilder.POST, getPhoneBookEntryUrl(entryId));
        reqBuilder.setHeader(CONTENT_TYPE, VARIANT);
        try {
            reqBuilder.sendRequest(buildJsonRequest(vm).toString(), new RequestCallback() {

                @Override
                public void onResponseReceived(Request request, Response response) {
                    int httpStatusCode = response.getStatusCode();
                    String message = s_searchConstants.addPhonebookEntryFailed();
                    if (httpStatusCode == PHONEBOOK_DUPLICATE_ENTRY_ERROR) {
                        message = s_searchConstants.duplicatePhonebookEntry();
                    } else if (httpStatusCode == Response.SC_OK) {
                        message = successMessage;
                    }
                    SC.say(message);
                }

                @Override
                public void onError(Request request, Throwable exception) {
                    SC.warn(s_searchConstants.addPhonebookEntryFailed());
                }
            });
        } catch (RequestException ex) {
            SC.warn(s_searchConstants.addPhonebookEntryFailed());
        }
    }

    public JSONObject buildJsonRequest(ValuesManager form) {
        JSONObject entry = new JSONObject();
        addToJsonObject(entry, FIRST_NAME, form.getValueAsString(FIRST_NAME));
        addToJsonObject(entry, LAST_NAME, form.getValueAsString(LAST_NAME));
        addToJsonObject(entry, NUMBER, form.getValueAsString(NUMBER));

        JSONObject contactInfo = new JSONObject();
        addToJsonObject(contactInfo, JOB_TITLE, form.getValueAsString(JOB_TITLE));
        addToJsonObject(contactInfo, JOB_DEPT, form.getValueAsString(JOB_DEPT));
        addToJsonObject(contactInfo, COMPANY_NAME, form.getValueAsString(COMPANY_NAME));
        addToJsonObject(contactInfo, ASSISTANT_NAME, form.getValueAsString(ASSISTANT_NAME));
        addToJsonObject(contactInfo, CELL_PHONE_NUMBER, form.getValueAsString(CELL_PHONE_NUMBER));
        addToJsonObject(contactInfo, HOME_PHONE_NUMBER, form.getValueAsString(HOME_PHONE_NUMBER));
        addToJsonObject(contactInfo, ASSISTANT_PHONE_NUMBER, form.getValueAsString(ASSISTANT_PHONE_NUMBER));
        addToJsonObject(contactInfo, FAX_NUMBER, form.getValueAsString(FAX_NUMBER));
        addToJsonObject(contactInfo, IM_ID, form.getValueAsString(IM_ID));
        addToJsonObject(contactInfo, IM_DISPLAY_ID, form.getValueAsString(IM_DISPLAY_ID));
        addToJsonObject(contactInfo, ALTERNATE_IM_ID, form.getValueAsString(ALTERNATE_IM_ID));
        addToJsonObject(contactInfo, LOCATION, form.getValueAsString(LOCATION));
        addToJsonObject(contactInfo, EMAIL_ADDRESS, form.getValueAsString(EMAIL_ADDRESS));
        addToJsonObject(contactInfo, ALTERNATE_EMAIL_ADDRESS, form.getValueAsString(ALTERNATE_EMAIL_ADDRESS));

        JSONObject homeAddress = new JSONObject();
        addToJsonObject(homeAddress, ADDRESS_STREET, form.getValueAsString(HOME_STREET));
        addToJsonObject(homeAddress, ADDRESS_CITY, form.getValueAsString(HOME_CITY));
        addToJsonObject(homeAddress, ADDRESS_STATE, form.getValueAsString(HOME_STATE));
        addToJsonObject(homeAddress, ADDRESS_COUNTRY, form.getValueAsString(HOME_COUNTRY));
        addToJsonObject(homeAddress, ADDRESS_ZIP, form.getValueAsString(HOME_ZIP));
        contactInfo.put(HOME_ADDRESS, homeAddress);

        JSONObject officeAddress = new JSONObject();
        addToJsonObject(officeAddress, ADDRESS_STREET, form.getValueAsString(OFFICE_STREET));
        addToJsonObject(officeAddress, ADDRESS_CITY, form.getValueAsString(OFFICE_CITY));
        addToJsonObject(officeAddress, ADDRESS_STATE, form.getValueAsString(OFFICE_STATE));
        addToJsonObject(officeAddress, ADDRESS_COUNTRY, form.getValueAsString(OFFICE_COUNTRY));
        addToJsonObject(officeAddress, ADDRESS_ZIP, form.getValueAsString(OFFICE_ZIP));
        addToJsonObject(officeAddress, OFFICE_DESIGNATION, form.getValueAsString(OFFICE_DESIGNATION));
        contactInfo.put(OFFICE_ADDRESS, officeAddress);

        entry.put(PhonebookDataSource.CONTACT_INFORMATION, contactInfo);

        JSONObject clientRequestObject = new JSONObject();
        clientRequestObject.put(KEY, entry);
        return clientRequestObject;
    }

    private void addToJsonObject(JSONObject jsonObject, String key, String fieldValue) {
        if (fieldValue != null) {
            jsonObject.put(key, new JSONString(fieldValue));
        }
    }

    private String getPhoneBookEntryUrl(String id) {
        return "/sipxconfig/rest/my/phonebook/entry/" + id;
    }

    private String getValue(String value) {
        if (value == null || value.equals("null")) {
            return "";
        }
        return value;
    }

}
