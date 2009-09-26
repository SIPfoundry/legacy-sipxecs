/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.util.ListResourceBundle;
import java.util.ResourceBundle;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.freeswitch.TextToPrompts_en;

import junit.framework.TestCase;

public class PromptListTest extends TestCase {

    public class MyResources extends ListResourceBundle {
        private final Object[][] m_contents = {
            {
                "global.prefix", "glob/"
            }, {
                "test1.prompts", "woof"
            }, {
                "test2.prefix", "/top/"
            }, {
                "test2.prompts", "woof:was:here"
            }, {
                "test3.prompts", "{0,prompts}:was:here"
            }, {
                "test4.prefix", "pre"
            }, {
                "test4.prompts", "{0,prompts}:was:here"
            }, {
                "test5.prompts", "{0}:{1}"
            }, {
                "test6.prompts", "/joe:{0,ordinal}:dogs"
            },
        };

        public Object[][] getContents() {
            return m_contents;
        }

    }

    public void testAddFragment() {
        ResourceBundle rb = new MyResources();
        TextToPrompts ttp = new TextToPrompts_en();
        ttp.setPrefix("/TTP");

        PromptList pl = new PromptList(rb, null, ttp);
        pl.addFragment("test1");
        assertEquals("glob/woof", pl.toString());

        pl = new PromptList(rb, null, ttp);
        pl.addFragment("test2");
        assertEquals("/top/woof:/top/was:/top/here", pl.toString());

        pl = new PromptList(rb, null, ttp);
        pl.addFragment("test3", "fuzzy");
        // NB variable prompts do NOT get prefixed!
        assertEquals("fuzzy:glob/was:glob/here", pl.toString());

        pl = new PromptList(rb, null, ttp);
        pl.addFragment("test4", "fuzzy:wuzzy");
        assertEquals("fuzzy:wuzzy:pre/was:pre/here", pl.toString());

        pl = new PromptList(rb, null, ttp);
        pl.addFragment("test5", "42", "0");
        assertEquals("/TTP/42.wav:/TTP/0.wav", pl.toString());

        pl = new PromptList(rb, null, ttp);
        pl.setPrefix("/PROMPT");
        pl.addFragment("test6", "2");
        assertEquals("/joe:/TTP/2nd.wav:/PROMPT/dogs", pl.toString());
    }

    public void testAppendPrefix() {
        PromptList pl = new PromptList();
        assertEquals("/dog", pl.appendPrefix("/foo/", "/dog"));
        assertEquals("/foo/dog", pl.appendPrefix("/foo/", "dog"));
        assertEquals("dog", pl.appendPrefix(null, "dog"));
        pl.setPrefix("/cat");
        assertEquals("/cat/dog", pl.appendPrefix(null, "dog"));
    }

    public void testAddPrompts() {
        ResourceBundle rb = new MyResources();
        TextToPrompts ttp = new TextToPrompts_en();
        ttp.setPrefix("/TTP");

        PromptList pl = new PromptList(rb, null, ttp);
        pl.addFragment("test1");
        assertEquals("glob/woof", pl.toString());

        PromptList pl2 = new PromptList(rb, null, ttp);
        pl2.addPrompts("fuzzy");
        assertEquals("glob/fuzzy", pl2.toString());
        pl2.addPrompts(pl);
        assertEquals("glob/fuzzy:glob/woof", pl2.toString());
    }

    public void testAddPromptsUrl() {
        ResourceBundle rb = new MyResources();
        TextToPrompts ttp = new TextToPrompts_en();
        ttp.setPrefix("/TTP");

        PromptList pl = new PromptList(rb, null, ttp);
        pl.addFragment("test1");
        assertEquals("glob/woof", pl.toString());

        PromptList pl2 = new PromptList(rb, null, ttp);
        pl2.addUrl("fuzzy://dog");
        assertEquals("fuzzy://dog", pl2.toString());
        pl2.addPrompts(pl);
        assertEquals("fuzzy://dog:glob/woof", pl2.toString());
    }

}
