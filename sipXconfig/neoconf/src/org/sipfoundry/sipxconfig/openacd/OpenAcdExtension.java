/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class OpenAcdExtension extends FreeswitchExtension implements Replicable, DeployConfigOnEdit {
    public static final String DESTINATION_NUMBER = "destination_number";
    public static final String DESTINATION_NUMBER_PATTERN = "^%s$";
    public static final String EMPTY_STRING = "";
    public static final String VALID_REGULAR_EXPRESSION = "^\\((\\d+)\\).*$";
    static final String ALIAS_RELATION = "openacd";
    private AddressManager m_addressManager;

    /**
     * We call this condition the (first, because they can be many) condition that has
     * destination_number as a field
     */
    public FreeswitchCondition getNumberCondition() {
        if (getConditions() == null) {
            return null;
        }
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                return condition;
            }
        }
        return null;
    }

    public String getExtension() {
        if (getNumberCondition() != null) {
            return getNumberCondition().getExtension();
        }
        return null;
    }

    //We want to allow regular expressions to be used in extension field.
    //The regex pattern must start with 1 capture group and that capture group to be
    //the real extension to be sent to the Freeswitch node in the dial string.
    //We will validate: the regular expression to start with 1 group and the capture group to be
    //a valid extension. That extension we will validate as alias.
    //we will validate it only if the user will check the extension as a regular expression.
    public String getCapturedExtension() {
        String extension = getExtension();
        if (extension == null) {
            return null;
        }
        String validPhonePattern = "[\\d*]+";
        if (getNumberCondition().isRegex()) {
            //is the extension a valid regular expression?
            try {
                Pattern isValidPattern = Pattern.compile(extension);
            } catch (PatternSyntaxException e) {
                throw new UserException("&error.regex.invalid");
            }
            Pattern validRegex = Pattern.compile(VALID_REGULAR_EXPRESSION);
            Matcher m = validRegex.matcher(extension);
            if (!m.matches()) {
                throw new UserException("&error.regex.no.valid.group");
            }
            //we are sure there's a capturing group with index 1
            //otherwise, the previous match would fail
            extension = m.group(1);
        } else {
            if (!Pattern.matches(validPhonePattern, extension)) {
                throw new UserException("&error.validPhone");
            }
        }
        return extension;
    }

    public boolean getRegex() {
        if (getNumberCondition() != null) {
            return getNumberCondition().isRegex();
        }
        return false;
    }

    public List<FreeswitchAction> getLineActions() {
        List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
        for (FreeswitchCondition condition : getConditions()) {
            if (condition.getField().equals(DESTINATION_NUMBER)) {
                for (FreeswitchAction action : condition.getActions()) {
                    actions.add(action);
                }
            }
        }
        return actions;
    }

    public static FreeswitchAction createAction(String application, String data) {
        FreeswitchAction action = new FreeswitchAction();
        action.setApplication(application);
        action.setData(data);
        return action;
    }

    public static FreeswitchCondition createLineCondition() {
        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField(OpenAcdLine.DESTINATION_NUMBER);
        condition.setExpression(EMPTY_STRING);
        return condition;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        Address address = m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        String extension = getCapturedExtension();
        String sipUri = SipUri.format(extension, address.getAddress(), address.getPort());
        String sipUriNoQuote = SipUri.format(extension, address.getAddress(), address.getPort(), false);
        AliasMapping nameMapping = new AliasMapping(getName(), sipUriNoQuote, ALIAS_RELATION);
        AliasMapping lineMapping = new AliasMapping(extension, sipUri, ALIAS_RELATION);
        mappings.addAll(Arrays.asList(nameMapping, lineMapping));
        if (getAlias() != null) {
            AliasMapping aliasMapping = new AliasMapping(getAlias(), sipUri, ALIAS_RELATION);
            mappings.add(aliasMapping);
        }
        if (getDid() != null) {
            AliasMapping didMapping = new AliasMapping(getDid(), sipUri, ALIAS_RELATION);
            mappings.add(didMapping);
        }

        return mappings;
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.ALIAS);
        return ds;
    }

    @Override
    public String getIdentity(String domain) {
        return SipUri.stripSipPrefix(SipUri.format(null, getExtension(), domain));
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.emptyMap();
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FreeswitchFeature.FEATURE);
    }
}
