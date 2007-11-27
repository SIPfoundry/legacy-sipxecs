package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.springframework.context.ApplicationContext;

public class DataSetGeneratorTestIntegration extends IntegrationTestCase {
    
    private Map<String, DataSetGenerator> m_dataSetGeneratorMap;
    
    public DataSetGeneratorTestIntegration() {
        m_dataSetGeneratorMap = new HashMap<String, DataSetGenerator>();
    }
    
    public void testDataSetGeneratorBeans() {
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet set = (DataSet) i.next();
            // need to fix this test.  caller-alias bean can not be injected
            // automatically (setter method is commented out below)
            if ("caller-alias".equals(set.getName())) {
                continue;
            }
            DataSetGenerator gen = (DataSetGenerator) m_dataSetGeneratorMap.get(set.getName());
            assertNotNull(set.getName(), gen);
            assertNotNull(set.getName(), gen.getCoreContext());
        }
    }
    
    public void setExtensionDataSet(Extensions extensions) {
        m_dataSetGeneratorMap.put(DataSet.EXTENSION.getName(), extensions);
    }
    
    public void setCredentialDataSet(Credentials credentials) {
        m_dataSetGeneratorMap.put(DataSet.CREDENTIAL.getName(), credentials);
    }
    
    /*public void setCallerAliasDataSet(CallerAliases callerAliases) {
        m_DataSetGeneratorMap.put(DataSet.CALLER_ALIAS.getName(), callerAliases);
    }*/
    
    public void setAliasDataSet(Aliases aliases) {
        m_dataSetGeneratorMap.put(DataSet.ALIAS.getName(), aliases);
    }
    
    public void setPermissionDataSet(Permissions permissions) {
        m_dataSetGeneratorMap.put(DataSet.PERMISSION.getName(), permissions);
    }
}
