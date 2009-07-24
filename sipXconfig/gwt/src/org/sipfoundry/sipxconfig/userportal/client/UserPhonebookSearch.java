/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.userportal.client;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.MultiWordSuggestOracle;
import com.google.gwt.user.client.ui.RootPanel;
import com.google.gwt.user.client.ui.SuggestBox;
import com.google.gwt.user.client.ui.VerticalPanel;


public class UserPhonebookSearch implements EntryPoint {

    @Override
    public void onModuleLoad() {
        // Define the oracle that finds suggestions
        MultiWordSuggestOracle oracle = new MultiWordSuggestOracle();
        String[] words = {"200", "201", "123"};
        for (int i = 0; i < words.length; ++i) {
            oracle.add(words[i]);
        }

        // Create the suggest box
        final SuggestBox suggestBox = new SuggestBox(oracle);
        VerticalPanel suggestPanel = new VerticalPanel();
        suggestPanel.add(new Label("Search"));
        suggestPanel.add(suggestBox);
        suggestPanel.addStyleName("demo-panel-padded");
        RootPanel.get("user_phonebook_search").add(suggestPanel);
    }

}
