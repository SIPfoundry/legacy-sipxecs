/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.ivr;

import static java.lang.String.format;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collection;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.springframework.beans.factory.annotation.Required;

public class IvrConfig implements ConfigProvider {
    private Ivr m_ivr;

//    <property name="processName" value="sipXivr" />
//    <property name="modelName" value="sipxivr.xml" />
//    <property name="modelDir" value="sipxivr" />
//    <property name="editable" value="true" />
//    <property name="groupTitleEnabled" value="true" />
//    <property name="callPilotSettings" value="nameDialPrefix, defaultTui"/>
//    <property name="cpuiDir" value="${sysdir.doc}/stdprompts/cpui"/>
//    <property name="configurations">
//      <list>
//        <ref bean="sipxIvrConfiguration" />
//        <ref bean="autoAttendantsConfig" />
//        <ref bean="statusPluginConfiguration" />
//      </list>
//    </property>
//    <property name="mailstoreDir" value="${sysdir.mailstore}" />
//    <property name="promptsDir" value="${sysdir.vxml.prompts}" />
//    <property name="scriptsDir" value="${sysdir.vxml.scripts}" />
//    <property name="docDir" value="${sysdir.doc}" />
//    <property name="vxmlDir" value="${sysdir.vxml}" />
//    <property name="binDir" value="${sysdir.bin}" />
//    <property name="backupPath" value="${sysdir.var}/backup" />
//    <property name="bundles">
//      <set>
//        <ref bean="voicemailBundle" />
//      </set>
//    </property>

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(DialPlanContext.FEATURE, Ivr.FEATURE)) {
            Collection<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(Ivr.FEATURE);
            Address mwiApi = manager.getAddressManager().getSingleAddress(Mwi.HTTP_API);
            Address adminApi = manager.getAddressManager().getSingleAddress(AdminContext.HTTPS_ADDRESS);
            Address restApi = manager.getAddressManager().getSingleAddress(Rest.API_ADDRESS);
            Address imApi = manager.getAddressManager().getSingleAddress(ImManager.API_ADDRESS);
            for (Location location : locations) {
                File f = new File(manager.getLocationDataDirectory(location), "sipxivr.properties.cfdat");
                FileWriter wtr = new FileWriter(f);
                KeyValueConfiguration config = new KeyValueConfiguration(wtr, "=");
                IvrSettings settings = m_ivr.getSettings(); 
                config.write(settings.getSettings());
                DomainManager dm = manager.getDomainManager();
                config.write("ivr.operatorAddr", "sip:operator@" + dm.getDomainName());
                config.write("ivr.sipxchangeDomainName", dm.getDomainName());
                config.write("ivr.realm", dm.getAuthorizationRealm());
                String mwi = format("https://%s:%d/cgi/StatusEvent.cgi", mwiAddr.getAddress(), mwiAddr.getPort());
                config.write("ivr.mwiUrl", mwi);
//                
//                String rest
//                    
//                ivr.configUrl=https://${configService.fqdn}:8443
//                ivr.3pccSecureUrl=https://$!{restService.fqdn}:$!{restService.httpsPort}
//                ivr.callHistoryUrl=https://$!{restService.fqdn}:$!{restService.httpsPort}/cdr/
//                ivr.sipxSupervisorHost=${location.fqdn}
//                ivr.sipxSupervisorXmlRpcPort=${sipxSupervisorXmlRpcPort}
//                
//                #if (${sipxServiceManager.isServiceInstalled('sipxOpenfireService')})
//                ivr.openfireHost=$!{sipxServiceManager.getServiceParam('openfire-host')}
//                ivr.openfireXmlRpcPort=$!{sipxServiceManager.getServiceParam('openfire-xml-rpc-port')}
//                #if (${sipxServiceManager.getServiceByBeanId('sipxImbotService').isAvailable()})
//                ivr.sendIMUrl=http://$!{sipxServiceManager.getServiceParam('openfire-host')}:$!{imbotService.httpPort}/IM
//                #end
//                #end
//                ivr.configAddress=${configService.address}
                
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    @Required
    public void setIvr(Ivr ivr) {
        m_ivr = ivr;
    }
}
