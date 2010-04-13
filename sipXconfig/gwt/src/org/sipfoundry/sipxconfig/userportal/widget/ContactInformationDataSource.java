/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.widget;

import com.google.gwt.json.client.JSONObject;
import com.google.gwt.json.client.JSONString;
import com.smartgwt.client.data.DataSource;
import com.smartgwt.client.data.DataSourceField;
import com.smartgwt.client.data.fields.DataSourceBooleanField;
import com.smartgwt.client.data.fields.DataSourceTextField;
import com.smartgwt.client.widgets.form.ValuesManager;

import org.sipfoundry.sipxconfig.userportal.client.HttpRequestBuilder;


public class ContactInformationDataSource extends DataSource {

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
    public static final String USE_BRANCH_ADDRESS = "useBranchAddress";

    public static final String EMAIL_ADDRESS = "emailAddress";
    public static final String ALTERNATE_EMAIL_ADDRESS = "alternateEmailAddress";

    public static final String[] FIELDS_GENERAL = {
        EMAIL_ADDRESS, LOCATION, ALTERNATE_EMAIL_ADDRESS, JOB_TITLE,
        CELL_PHONE_NUMBER, IM_ID, JOB_DEPT, FAX_NUMBER, ALTERNATE_IM_ID, COMPANY_NAME
    };

    public static final String[] FIELDS_HOME = {
        HOME_STREET, HOME_STATE, HOME_ZIP, HOME_CITY, HOME_COUNTRY, HOME_PHONE_NUMBER
    };

    public static final String[] FIELDS_OFFICE = {
        OFFICE_DESIGNATION, OFFICE_STREET, OFFICE_COUNTRY, ASSISTANT_NAME, OFFICE_CITY,
        OFFICE_ZIP, ASSISTANT_PHONE_NUMBER, OFFICE_STATE
    };

    private static final String REST_URL = "/sipxconfig/rest/my/contact-information/";

    private static final String ADDRESS_STREET = "street";
    private static final String ADDRESS_CITY = "city";
    private static final String ADDRESS_STATE = "state";
    private static final String ADDRESS_COUNTRY = "country";
    private static final String ADDRESS_ZIP = "zip";
    private static final String KEY = "contact-information";
    private static final String VARIANT = "application/json";
    private static final String CONTENT_TYPE = "Content-Type";


    public ContactInformationDataSource(String id) {
        super();
        setID(id);

        DataSourceField emailAddress = new DataSourceTextField(EMAIL_ADDRESS);
        emailAddress.setValueXPath(EMAIL_ADDRESS);

        DataSourceField jobTitle = new DataSourceTextField(JOB_TITLE);
        jobTitle.setValueXPath(JOB_TITLE);
        DataSourceField jobDept = new DataSourceTextField(JOB_DEPT);
        jobDept.setValueXPath(JOB_DEPT);
        DataSourceField companyName = new DataSourceTextField(COMPANY_NAME);
        companyName.setValueXPath(COMPANY_NAME);
        DataSourceField assistantName = new DataSourceTextField(ASSISTANT_NAME);
        assistantName.setValueXPath(ASSISTANT_NAME);
        DataSourceField location = new DataSourceTextField(LOCATION);
        location.setValueXPath(LOCATION);
        DataSourceField cellPhoneNumber = new DataSourceTextField(CELL_PHONE_NUMBER);
        cellPhoneNumber.setValueXPath(CELL_PHONE_NUMBER);
        DataSourceField homePhoneNumber = new DataSourceTextField(HOME_PHONE_NUMBER);
        homePhoneNumber.setValueXPath(HOME_PHONE_NUMBER);
        DataSourceField assistantPhoneNumber = new DataSourceTextField(ASSISTANT_PHONE_NUMBER);
        assistantPhoneNumber.setValueXPath(ASSISTANT_PHONE_NUMBER);
        DataSourceField faxNumber = new DataSourceTextField(FAX_NUMBER);
        faxNumber.setValueXPath(FAX_NUMBER);
        DataSourceField imId = new DataSourceTextField(IM_ID);
        imId.setValueXPath(IM_ID);
        DataSourceField alternateImId = new DataSourceTextField(ALTERNATE_IM_ID);
        alternateImId.setValueXPath(ALTERNATE_IM_ID);
        DataSourceField alternateEmailAddress = new DataSourceTextField(ALTERNATE_EMAIL_ADDRESS);
        alternateEmailAddress.setValueXPath(ALTERNATE_EMAIL_ADDRESS);

        DataSourceField homeStreet = new DataSourceTextField(HOME_STREET);
        homeStreet.setValueXPath("homeAddress/street");
        DataSourceField homeCity = new DataSourceTextField(HOME_CITY);
        homeCity.setValueXPath("homeAddress/city");
        DataSourceField homeCountry = new DataSourceTextField(HOME_COUNTRY);
        homeCountry.setValueXPath("homeAddress/country");
        DataSourceField homeState = new DataSourceTextField(HOME_STATE);
        homeState.setValueXPath("homeAddress/state");
        DataSourceField homeZip = new DataSourceTextField(HOME_ZIP);
        homeZip.setValueXPath("homeAddress/zip");

        DataSourceField officeStreet = new DataSourceTextField(OFFICE_STREET);
        officeStreet.setValueXPath("officeAddress/street");
        DataSourceField officeCity = new DataSourceTextField(OFFICE_CITY);
        officeCity.setValueXPath("officeAddress/city");
        DataSourceField officeCountry = new DataSourceTextField(OFFICE_COUNTRY);
        officeCountry.setValueXPath("officeAddress/country");
        DataSourceField officeState = new DataSourceTextField(OFFICE_STATE);
        officeState.setValueXPath("officeAddress/state");
        DataSourceField officeZip = new DataSourceTextField(OFFICE_ZIP);
        officeZip.setValueXPath("officeAddress/zip");
        DataSourceField officeDesignation = new DataSourceTextField(OFFICE_DESIGNATION);
        officeDesignation.setValueXPath("officeAddress/officeDesignation");

        DataSourceField useBranchAddress = new DataSourceBooleanField(USE_BRANCH_ADDRESS);
        useBranchAddress.setValueXPath(USE_BRANCH_ADDRESS);

        DataSourceField avatar = new DataSourceTextField(AVATAR);
        avatar.setValueXPath(AVATAR);

        setFields(emailAddress, jobTitle, jobDept, companyName, assistantName, location, cellPhoneNumber,
                homePhoneNumber, assistantPhoneNumber, faxNumber, imId, alternateImId,
                alternateEmailAddress, homeStreet, homeCity, homeCountry, homeState, homeZip, officeStreet,
                officeCity, officeCountry, officeState, officeZip, officeDesignation, avatar, useBranchAddress);
        setRecordXPath("/contact-information");
        setDataURL(REST_URL);

        /*
         * NOTE -
         * ECLIPSE DEBUGGING
         *
         * REST URL is not accessible when debugging the project from inside eclipse. You can use the data URL
         * "ContactInformationTestData.xml" instead. It will give you a set of test data to work with.
         *
         * setDataURL("ContactInformationTestData.xml");
         */
        setClientOnly(false);
    }

    public void editEntry(ValuesManager vm) {
        HttpRequestBuilder.doPut(REST_URL, buildJsonRequest(vm).toString(), CONTENT_TYPE,
                VARIANT);
    }

    public JSONObject buildJsonRequest(ValuesManager form) {
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
        addToJsonObject(contactInfo, USE_BRANCH_ADDRESS, form.getValueAsString(USE_BRANCH_ADDRESS));

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

        JSONObject clientRequestObject = new JSONObject();
        clientRequestObject.put(KEY, contactInfo);
        return clientRequestObject;
    }

    private void addToJsonObject(JSONObject jsonObject, String key, String fieldValue) {
        if (fieldValue != null) {
            jsonObject.put(key, new JSONString(fieldValue));
        }
    }
}
