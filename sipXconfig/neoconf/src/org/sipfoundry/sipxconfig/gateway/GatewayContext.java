/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import java.util.Collection;
import java.util.List;

public interface GatewayContext {
    public static final String CONTEXT_BEAN_NAME = "gatewayContext";

    List<Gateway> getGateways();

    Collection<Integer> getAllGatewayIds();

    List<Gateway> getGatewayByIds(Collection<Integer> gatewayIds);

    <T> List< ? extends T> getGatewayByType(Class<T> type);

    Gateway getGateway(Integer id);

    FxoPort getPort(Integer id);

    void storeGateway(Gateway gateway);

    void storePort(FxoPort port);

    void clear();

    boolean deleteGateway(Integer id);

    void deleteGateways(Collection<Integer> selectedRows);

    /**
     * Returns the list of gateways available for a specific rule
     *
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    Collection<Gateway> getAvailableGateways(Integer ruleId);

    void addGatewaysToRule(Integer dialRuleIs, Collection<Integer> gatewaysIds);

    void removeGatewaysFromRule(Integer dialRuleIs, Collection<Integer> gatewaysIds);

    Gateway newGateway(GatewayModel model);

    void removePortsFromGateway(Integer gatewayId, Collection<Integer> portIds);

    Integer getGatewayIdBySerialNumber(String serialNumber);
}
