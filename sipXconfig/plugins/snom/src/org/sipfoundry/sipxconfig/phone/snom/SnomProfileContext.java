/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.snom;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.lang.ArrayUtils;
import org.apache.velocity.tools.generic.EscapeTool;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class SnomProfileContext extends ProfileContext<SnomPhone> {

    private static final int PHONEBOOK_MAX = 100;
    private static final int SPEEDDIAL_MAX = 33;

    private final SpeedDial m_speedDial;
    private final Collection<PhonebookEntry> m_phoneBook;

    public SnomProfileContext(SnomPhone device, SpeedDial speedDial, Collection<PhonebookEntry> phoneBook,
            String profileTemplate) {
        super(device, profileTemplate);
        m_speedDial = speedDial;
        m_phoneBook = trim(phoneBook);
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        context.put("speedDial", getNumbers());
        context.put("phoneBook", m_phoneBook);
        context.put("esc", new EscapeTool());
        return context;
    }

    Collection<PhonebookEntry> trim(Collection<PhonebookEntry> phoneBook) {
        if (phoneBook == null) {
            return Collections.emptyList();
        }
        if (phoneBook.size() <= PHONEBOOK_MAX) {
            return phoneBook;
        }
        Predicate keepOnlyN = new Predicate() {
            private int m_i;

            public boolean evaluate(Object arg0) {
                return m_i++ < PHONEBOOK_MAX;
            }
        };
        CollectionUtils.filter(phoneBook, keepOnlyN);
        return phoneBook;
    }

    /**
     * Create SNOM speed dial information
     *
     * @return and array of at most 33 (0-32) phone numbers
     */
    String[] getNumbers() {
        if (m_speedDial == null) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        List<Button> buttons = m_speedDial.getButtons();
        int size = Math.min(SPEEDDIAL_MAX, buttons.size());
        String[] numbers = new String[size];
        for (int i = 0; i < numbers.length; i++) {
            numbers[i] = buttons.get(i).getNumber();
        }
        return numbers;
    }
}
