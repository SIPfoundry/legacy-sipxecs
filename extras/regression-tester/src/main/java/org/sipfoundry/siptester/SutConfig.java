package org.sipfoundry.siptester;

import java.util.HashSet;

public class SutConfig {
      
    HashSet<SutUA> sutUACollection = new HashSet<SutUA>();
    
    
    public void addSutUA(SutUA sutUa) {
        this.sutUACollection.add(sutUa);
    }
    
    public HashSet<SutUA> getSutUACollection() {
        return this.sutUACollection;
    }
    
   

}
