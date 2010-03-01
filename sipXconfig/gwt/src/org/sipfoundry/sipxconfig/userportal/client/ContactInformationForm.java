/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import com.google.gwt.i18n.client.ConstantsWithLookup;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.fields.FormItem;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.validator.RegExpValidator;
import com.smartgwt.client.widgets.form.validator.RequiredIfFunction;
import com.smartgwt.client.widgets.form.validator.RequiredIfValidator;
import com.smartgwt.client.widgets.grid.ListGridRecord;

public class ContactInformationForm extends DynamicForm {

    private static final String TITLE_STYLE = "titleFormStyle";
    private static final String EMAIL_EXPRESSION = "^([a-zA-Z0-9_.\\-+])+@(([a-zA-Z0-9\\-])+\\.)+[a-zA-Z0-9]{2,4}$";

    private ConstantsWithLookup m_constants;

    public ContactInformationForm(String[] fieldNames, String formName, int numCols, ConstantsWithLookup constants) {
        m_constants = constants;
        setGroupTitle(m_constants.getString(formName));
        setIsGroup(true);
        setNumCols(numCols);
        setFixedColWidths(true);
        setFields(createFields(fieldNames, m_constants));
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
            item.setValue("");
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
        requiredValidator.setErrorMessage(m_constants.getString("requiredField"));
        item.setValidators(requiredValidator);
    }

    public void addEmailValidator(String name) {
        FormItem item = getItem(name);
        RegExpValidator emailValidator = new RegExpValidator();
        emailValidator.setErrorMessage(m_constants.getString("invalidEmail"));
        emailValidator.setExpression(EMAIL_EXPRESSION);
        item.setValidators(emailValidator);
    }

}
