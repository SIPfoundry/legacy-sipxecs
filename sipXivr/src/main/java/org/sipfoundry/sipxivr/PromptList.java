/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.util.ArrayList;
import java.util.ResourceBundle;

public class PromptList {

    class PromptGroup {
        private String m_prefix = "";
        private ArrayList<String> m_prompts = new ArrayList<String>();
        private String[] m_variables = {};
    }

    private ArrayList<PromptGroup> m_promptGroups = new ArrayList<PromptGroup>();
    private ResourceBundle m_resourceBundle;
    private TextToPrompts m_ttp;

    private String m_globalPrefix = "";

    public PromptList() {
    }

    public PromptList(ResourceBundle resourceBundle, Configuration config, TextToPrompts ttp) {
        setResourceBundle(resourceBundle, config);
        setTtp(ttp);
    }

    public void setResourceBundle(ResourceBundle resourceBundle, Configuration config) {
        this.m_resourceBundle = resourceBundle;
        // Try setting the global prefix
        try {
            m_globalPrefix = resourceBundle.getString("global.prefix");
            if (!m_globalPrefix.startsWith("/") && config != null) {
            	String docDir = config.getDocDirectory();
            	if (!docDir.endsWith("/")) {
            		docDir += "/";
            	}
            	m_globalPrefix = docDir + m_globalPrefix;
            }
            if (!m_globalPrefix.endsWith("/")) {
                m_globalPrefix += "/";
            }
        } catch (Exception e) {
            // Nothing to do, no where to go home...
        }
    }

    public void setTtp(TextToPrompts ttp) {
        this.m_ttp = ttp;
    }

    public void setPrefix(String prefix) {
        if (!prefix.endsWith("/")) {
            prefix += "/";
        }
        this.m_globalPrefix = prefix;
    }

    String appendPrefix(String localPrefix, String prompt) {
        String usePrefix = localPrefix != null ? localPrefix : m_globalPrefix;
        
        // If no prefix, or if rooted, don't prepend prefix
        if (usePrefix == null || prompt.startsWith("/") || prompt.startsWith(".") || prompt.startsWith("{")) {
            return prompt;
        }
        return usePrefix + prompt;
    }

    String renderVariable(String prompt, String... vars) {
        // Parse {position[,type[,format]]}
        String[] s = prompt.replace("{", "").replace("}", "").split(",");
        int position = 0;
        String type = "cardinal";
        String format = "";

        try {
            position = Integer.parseInt(s[0]);
            type = s[1].trim();
            format = s[2].trim();
        } catch (Throwable t) {
            // Nothing to do, no where to go home...
        }

        m_ttp.setType(type);
        m_ttp.setFormat(format);
        return m_ttp.render(vars[position]);
    }

    PromptGroup makePromptGroup(String localPrefix, String prompts, String... vars) {
        PromptGroup pg = new PromptGroup();
        pg.m_prefix = localPrefix;
        for (String prompt : prompts.split(":")) {
            // Found a variable reference. Render it
            if (prompt.startsWith("{")) {
                // Replace the reference with bunch of prompts
                for (String r : renderVariable(prompt, vars).split(":")) {
                    // Variables do NOT get prefixed
                    pg.m_prompts.add(r);
                }
            } else {
                pg.m_prompts.add(appendPrefix(localPrefix, prompt));
            }
        }
        pg.m_variables = vars;
        return pg;
    }

    public void addPromptsPrefixed(String localPrefix, String prompts, String... vars) {
        if (prompts != null) {
            this.m_promptGroups.add(makePromptGroup(localPrefix, prompts, vars));
        }
    }

    public void addPrompts(String prompts, String... vars) {
        addPromptsPrefixed(null, prompts, vars);
    }

    public void addPromptsPrefixed(String localPrefix, int index, String prompts, String... vars) {
        if (prompts != null) {
            this.m_promptGroups.add(index, makePromptGroup(localPrefix, prompts, vars));
        }
    }

    public void addPrompts(int index, String prompts, String... vars) {
        addPromptsPrefixed(null, index, prompts, vars);
    }

    public void addFragment(String fragment, String... vars) {
        // First see if the prompt has an prefix override
        String fragmentPrefix = null;
        try {
            fragmentPrefix = m_resourceBundle.getString(fragment + ".prefix");
            if (!fragmentPrefix.endsWith("/")) {
                fragmentPrefix += "/";
            }
        } catch (Exception e) {
            // Nothing to do, no where to go home...
        }

        addPromptsPrefixed(fragmentPrefix, m_resourceBundle.getString(fragment + ".prompts"), vars);
    }

    /**
     * Get the prompts as an ArrayList
     * 
     * @return
     */
    public ArrayList<String> getPrompts() {
        ArrayList<String> list = new ArrayList<String>();
        for (PromptGroup pg : m_promptGroups) {
            for (String string : pg.m_prompts) {
                list.add(string);
            }
        }
        return list;
    }

    /**
     * Get the prompts as a colon separated string
     * @return
     */
    public String toString() {
        StringBuilder result = new StringBuilder();
        for (PromptGroup pg : m_promptGroups) {
            for (String string : pg.m_prompts) {
                result.append(string).append(":");
            }
        }
        return result.deleteCharAt(result.length() - 1).toString();
    }

}
