/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.userportal.client;

import java.util.LinkedList;
import java.util.List;

import com.google.gwt.i18n.client.ConstantsWithLookup;
import com.smartgwt.client.widgets.form.DynamicForm;
import com.smartgwt.client.widgets.form.fields.FormItem;
import com.smartgwt.client.widgets.form.fields.StaticTextItem;
import com.smartgwt.client.widgets.form.fields.TextItem;
import com.smartgwt.client.widgets.form.validator.RegExpValidator;
import com.smartgwt.client.widgets.form.validator.RequiredIfFunction;
import com.smartgwt.client.widgets.form.validator.RequiredIfValidator;
import com.smartgwt.client.widgets.grid.ListGridRecord;

public class ContactInformationForm extends DynamicForm {

    private static final String EMPTY_STRING = "";
    private static final String TITLE_STYLE = "titleFormStyle";
    private static final String STATIC_TEXT_STYLE = "staticTextStyle";
    private static final String EMAIL_EXPRESSION = "^([a-zA-Z0-9_.\\-+])+@(([a-zA-Z0-9\\-])+\\.)+[a-zA-Z0-9]{2,4}$";

    private ConstantsWithLookup m_constants;
    private List<FormItem> m_textItems = new LinkedList<FormItem>();
    private List<FormItem> m_staticTextItems = new LinkedList<FormItem>();

    public ContactInformationForm(String[] fieldNames, String formName, int numCols, ConstantsWithLookup constants) {
        m_constants = constants;
        setGroupTitle(m_constants.getString(formName));
        setIsGroup(true);
        setNumCols(numCols);
        setFixedColWidths(true);
        setCanSelectText(true);
        createFields(fieldNames);
    }

    public void setData(ListGridRecord selection) {
        for (int i = 0; i < m_textItems.size(); i++) {
            FormItem textItem = m_textItems.get(i);
            String itemName = textItem.getName();
            String selectionValue = selection.getAttributeAsString(itemName);
            setItemData(textItem, selectionValue, false);

            FormItem staticTextItem = m_staticTextItems.get(i);
            setItemData(staticTextItem, selectionValue, true);
        }
    }

    private void setItemData(FormItem item, String value, boolean visible) {
        item.setValue(value);
        item.setTitleStyle(TITLE_STYLE);
        displayItem(item, visible);
    }

    public void editData() {
        for (FormItem item : m_textItems) {
            displayItem(item, true);
        }

        for (FormItem item : m_staticTextItems) {
            displayItem(item, false);
        }
    }

    public void maskData() {
        for (FormItem item : m_textItems) {
            item.disable();
        }
    }

    public void unMaskData() {
        for (FormItem item : m_textItems) {
            item.enable();
        }
    }

    public void addData() {
        for (FormItem item : m_textItems) {
            item.setValue(EMPTY_STRING);
            displayItem(item, true);
        }

        for (FormItem item : m_staticTextItems) {
            item.setValue(EMPTY_STRING);
            displayItem(item, false);
        }
    }

    private void displayItem(FormItem item, boolean show) {
        item.setVisible(show);
        item.redraw();
    }

    private void createFields(String[] fieldNames) {
        List<FormItem> items = new LinkedList<FormItem>();
        for (int i = 0; i < fieldNames.length; i++) {
            String title = m_constants.getString(fieldNames[i]);
            TextItem item = new TextItem(fieldNames[i], title);
            item.setTitleStyle(TITLE_STYLE);
            m_textItems.add(item);
            items.add(item);

            StaticTextItem staticTextItem = new StaticTextItem(fieldNames[i] + "_static", title);
            staticTextItem.setTitleStyle(TITLE_STYLE);
            staticTextItem.setVisible(false);
            staticTextItem.setCellStyle(STATIC_TEXT_STYLE);
            staticTextItem.setWrap(false);
            m_staticTextItems.add(staticTextItem);
            items.add(staticTextItem);
        }
        setItems(items.toArray(new FormItem[0]));
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
