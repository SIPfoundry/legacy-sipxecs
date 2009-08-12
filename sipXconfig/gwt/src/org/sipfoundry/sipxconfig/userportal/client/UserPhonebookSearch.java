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
import com.google.gwt.core.client.GWT;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.logical.shared.SelectionEvent;
import com.google.gwt.event.logical.shared.SelectionHandler;
import com.google.gwt.event.logical.shared.ValueChangeEvent;
import com.google.gwt.event.logical.shared.ValueChangeHandler;
import com.google.gwt.http.client.Request;
import com.google.gwt.http.client.RequestBuilder;
import com.google.gwt.http.client.RequestCallback;
import com.google.gwt.http.client.RequestException;
import com.google.gwt.http.client.Response;
import com.google.gwt.json.client.JSONArray;
import com.google.gwt.json.client.JSONObject;
import com.google.gwt.json.client.JSONParser;
import com.google.gwt.user.client.Window;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HasVerticalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.MultiWordSuggestOracle;
import com.google.gwt.user.client.ui.RootPanel;
import com.google.gwt.user.client.ui.SuggestBox;
import com.google.gwt.user.client.ui.SuggestOracle.Suggestion;
import com.google.gwt.user.client.ui.VerticalPanel;

public class UserPhonebookSearch implements EntryPoint {
    private static final String NUMBER = "number";
    private static final String FIRST_NAME = "first-name";
    private static final String LAST_NAME = "last-name";
    private static final String QUERY = "query";
    private static final String VALUE = "value";
    private static final String EMPTY = "";

    private final MultiWordSuggestOracle m_userList = new MultiWordSuggestOracle();
    private final SuggestBox m_suggestBox = new SuggestBox(m_userList);
    private final Button m_resetButton = new Button();

    @Override
    public void onModuleLoad() {

        doGet("/sipxconfig/rest/my/phonebook");
        SearchConstants searchConstants = (SearchConstants) GWT.create(SearchConstants.class);
        // Create the suggest box

        HorizontalPanel componentPanel = new HorizontalPanel();
        HTML title = new HTML(searchConstants.searchWidgetTitle());
        title.setStylePrimaryName("gwt-SipxSearchTitle");
        HTML description = new HTML(searchConstants.searchWidgetDescription());
        description.setStylePrimaryName("gwt-SipxSearchHelp");
        m_suggestBox.addValueChangeHandler(new SearchChangeListener());
        m_suggestBox.addSelectionHandler(new SearchSelectionListener());

        m_resetButton.setText(searchConstants.resetButton());
        m_resetButton.addClickHandler(new ClickListener());

        VerticalPanel titlePanel = new VerticalPanel();
        titlePanel.add(title);

        VerticalPanel suggestPanel = new VerticalPanel();
        HorizontalPanel widgetPanel = new HorizontalPanel();
        widgetPanel.add(m_suggestBox);
        widgetPanel.add(m_resetButton);
        suggestPanel.add(widgetPanel);
        suggestPanel.add(description);
        suggestPanel.addStyleName("demo-panel-padded");

        componentPanel.add(titlePanel);
        componentPanel.setSpacing(5);
        componentPanel.setCellVerticalAlignment(title, HasVerticalAlignment.ALIGN_MIDDLE);
        componentPanel.setCellHorizontalAlignment(title, HasHorizontalAlignment.ALIGN_LEFT);
        componentPanel.add(suggestPanel);
        componentPanel.setCellHorizontalAlignment(suggestPanel, HasHorizontalAlignment.ALIGN_LEFT);

        RootPanel.get("user_phonebook_search").add(componentPanel);
    }

    private void doGet(String url) {
        RequestBuilder builder = new RequestBuilder(RequestBuilder.GET, url);
        builder.setHeader("Accept", "application/json");
        try {
            builder.sendRequest(null, new RequestCallback() {
                public void onResponseReceived(Request request, Response response) {
                    JSONObject result = JSONParser.parse(response.getText()).isObject();
                    JSONArray usersListJSON = result.get("phonebook").isArray();
                    for (int i = 0; i < usersListJSON.size(); i++) {
                        JSONObject userJSON = usersListJSON.get(i).isObject();
                        if (userJSON.get(NUMBER) != null) {
                            m_userList.add(userJSON.get(NUMBER).isString().stringValue().trim());
                        }
                        if (userJSON.get(FIRST_NAME) != null) {
                            m_userList.add(userJSON.get(FIRST_NAME).isString().stringValue().trim());
                        }
                        if (userJSON.get(LAST_NAME) != null) {
                            m_userList.add(userJSON.get(LAST_NAME).isString().stringValue().trim());
                        }
                    }
                }

                public void onError(Request request, Throwable exception) {
                    // TODO: handle errors - warning on the screen
                }
            });
        } catch (RequestException e) {
            Window.alert(e.getMessage());
        }
    }

    public class SearchChangeListener implements ValueChangeHandler<String> {
        public void onValueChange(ValueChangeEvent<String> chEvent) {
            if (chEvent.getSource().equals(m_suggestBox)) {
                RootPanel.get(QUERY).getElement().setAttribute(VALUE, chEvent.getValue());
            }
        }
    }

    public class SearchSelectionListener implements SelectionHandler<Suggestion> {
        public void onSelection(SelectionEvent<Suggestion> selEvent) {
            if (selEvent.getSource().equals(m_suggestBox) && selEvent.getSelectedItem() != null) {
                RootPanel.get(QUERY).getElement().setAttribute(VALUE,
                        selEvent.getSelectedItem().getReplacementString());
            }
        }
    }

    public class ClickListener implements ClickHandler {
        public void onClick(ClickEvent event) {
            if (event.getSource().equals(m_resetButton)) {
                m_suggestBox.setText(EMPTY);
                RootPanel.get(QUERY).getElement().setAttribute(VALUE, EMPTY);
            }
        }
    }
}
