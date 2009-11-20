package com.pingtel.sipviewer;

import java.awt.Color;
import java.util.* ;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2002
 * Company:
 * @author
 * @version 1.0
 */

public class SIPChartModel
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected final static int MAX_KEYS = 50 ;
    protected final static int MAX_ENTRIES = 25000 ;
    protected final static String ERROR_KEY = "Error" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected String    m_trackableKeys[] ;
    protected double    m_trackableKeyPositions[];
    protected int       m_iNumKeys ;
    protected Hashtable m_htAliases ;
    protected ChartDescriptor   m_entries[] ;
    protected int       m_iNumEntries ;
    protected Vector    m_vListeners ;
    protected Hashtable m_htBranchIndex ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public SIPChartModel()
    {
        m_iNumKeys = 0 ;
        m_trackableKeys = new String[MAX_KEYS] ;
        m_trackableKeyPositions = new double[MAX_KEYS];
        
        m_iNumEntries = 0 ;
        m_entries = new ChartDescriptor[MAX_ENTRIES] ;
        m_htBranchIndex = new Hashtable(MAX_ENTRIES + 10) ;

        m_htAliases = new Hashtable(MAX_KEYS+10) ;
        m_vListeners = new Vector() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Method
////

    public void moveKeyLeft(String objTrackableKey)
    {
        String strTemp ;
        int col = findKeyColumn(objTrackableKey) ;
        if ((col >= 1) && (col < m_iNumKeys))
        {
            strTemp = m_trackableKeys[col-1] ;
            m_trackableKeys[col-1] = m_trackableKeys[col] ;
            m_trackableKeys[col] = strTemp ;

            reindexData() ;
            fireKeyMoved(col, col-1) ;
        }
    }


    public void moveKeyRight(String objTrackableKey)
    {
        String strTemp ;
        int col = findKeyColumn(objTrackableKey) ;
        if ((col >= 0) && ((col+1) < m_iNumKeys))
        {
            strTemp = m_trackableKeys[col+1] ;
            m_trackableKeys[col+1] = m_trackableKeys[col] ;
            m_trackableKeys[col] = strTemp ;

            reindexData() ;
            fireKeyMoved(col, col+1) ;
        }
    }


    // Check whether a key is in the table of keys (or aliases), and
    // if it is not there, add it.
    public void addKey(String objTrackableKey)
    {
        objTrackableKey = massageKey(objTrackableKey) ;

        if (findKey(objTrackableKey) == null)
        {
            if (m_iNumKeys < MAX_KEYS)
            {
                int iPosition = m_iNumKeys ;
                m_trackableKeys[m_iNumKeys++] = objTrackableKey ;
                fireKeyAdded(iPosition) ;
            }
            else
            {
                System.err.println("ERROR: Hit max number of keys: " + m_iNumKeys);
            }
        }
    }


    public void removeKey(String objTrackableKey)
    {
        objTrackableKey = massageKey(objTrackableKey) ;

        int iDeleteAt = -1 ;
        for (int i=0; i<m_iNumKeys; i++)
        {
            if (m_trackableKeys[i].equals(objTrackableKey))
            {
                iDeleteAt = i ;
            }
        }

        if (iDeleteAt != -1)
        {
            for (int i=iDeleteAt; i<m_iNumKeys; i++)
            {
                m_trackableKeys[i] = m_trackableKeys[i+1] ;
            }

            m_iNumKeys-- ;
            fireKeyDeleted(iDeleteAt) ;
        }
    }


    // Enter an alias in the table of aliases (m_htAliases) that translates
    // objAlias into objTrackableKey.
    public void addKeyAlias(String objTrackableKey, String objAlias)
    {
        objTrackableKey = massageKey(objTrackableKey) ;
        objAlias = massageKey(objAlias) ;

        // If the two keys are the same, do nothing.
        if (objTrackableKey.equals(objAlias))
        {
            return ;
        }

        // If the alias is already recorded (either as a key or an alias),
        // do nothing.
        if (findKey(objAlias) == null)
        {
            // Get the key that objAlias refers to, which may be different
            // if objAlias is itself an alias.
            String base = findKey(objTrackableKey);
            if (base == null)
            {
                addKey(objTrackableKey) ;
                base = objTrackableKey;
            }

            Vector vAliases = (Vector) m_htAliases.get(base) ;
            if (vAliases == null)
                vAliases = new Vector() ;

            vAliases.addElement(objAlias);

            m_htAliases.put(base, vAliases) ;
        }
    }


    public Vector getKeyAliases(String objTrackableKey)
    {
        objTrackableKey = massageKey(objTrackableKey) ;


        Vector vAliases = (Vector) m_htAliases.get(objTrackableKey) ;
        return vAliases ;
    }


    public String[] getKeys()
    {
        String strRC[] = new String[m_iNumKeys] ;
        for (int i=0; i<m_iNumKeys; i++)
        {
            strRC[i] = massageKey(m_trackableKeys[i].toString()) ;
        }

        return strRC ;
    }
    
    // return the array that holds the positions of all the vertical
    // columns, the positions are not absolute but rather a relative
    // % value from the left side of the main frame
    public double[] getKeyPositions()
    {
        return m_trackableKeyPositions;
    }


    public int getNumKeys()
    {
        return m_iNumKeys ;
    }


    public void addEntry(   String objTrackableSourceKey,
                            String objTrackableTargetKey,
                            String strLabel,
                            SipBranchData data)
    {
        objTrackableSourceKey = massageKey(objTrackableSourceKey) ;
        objTrackableTargetKey = massageKey(objTrackableTargetKey) ;
        String strBranchID = data.getThisBranchId() ;

        ChartDescriptor entry = new ChartDescriptor() ;
        if ((strBranchID != null) && (strBranchID.length() > 0))
        {
            if ((!data.isRequest()) && (data.getSourceEntity() == null))
            {
                Enumeration enumerator = getDescForBranch(strBranchID) ;
                if (enumerator != null)
                {
                    while (enumerator.hasMoreElements())
                    {
                        ChartDescriptor desc = (ChartDescriptor) enumerator.nextElement() ;
                        if (desc.dataSource.isRequest())
                        {
                            addKeyAlias(desc.dataSource.getDestinationEntity(), objTrackableSourceKey) ;
                        }
                    }
                }
            }
        }

        addKey(objTrackableSourceKey) ;
        addKey(objTrackableTargetKey) ;

        entry.source = objTrackableSourceKey ;
        entry.sourceColumn = findKeyColumn(objTrackableSourceKey) ;
        entry.target = objTrackableTargetKey ;
        entry.targetColumn = findKeyColumn(objTrackableTargetKey) ;
        entry.label = strLabel ;
        entry.dataSource = data ;
        entry.backgroundColor = Color.BLACK;

        if (entry.sourceColumn == -1)
        {
            System.err.println("ERROR: Cannot locate column for source '" +
                               entry.source + "' in frame " +
                               data.getFrameId());
            throw new IllegalArgumentException() ;
        }
        if (entry.targetColumn == -1)
        {
            System.err.println("ERROR: Cannot locate column for target '" +
                               entry.target + "' in frame " +
                               data.getFrameId());
            throw new IllegalArgumentException() ;
        }

        if (m_iNumEntries <  MAX_ENTRIES)
        {
            int iPosition = m_iNumEntries ;
            m_entries[m_iNumEntries++] = entry ;
            fireEntryAdded(iPosition, iPosition) ;
        }
        else
        {
            System.err.println("ERROR: Hit max number of entries: " + m_iNumEntries);
        }


        if ((strBranchID != null) && (strBranchID.length() > 0))
        {
            addBranchIndex(strBranchID, entry) ;
        }
    }


    public void clear()
    {
        int endingIndex = m_iNumEntries ;
        if (endingIndex > 0)
        {
            endingIndex-- ;
            m_iNumEntries = 0;

            fireEntryDeleted(0, endingIndex) ;
        }


        endingIndex = m_iNumKeys ;
        m_iNumKeys = 0 ;
        for (int i=0; i<endingIndex; i++)
        {
            fireKeyDeleted(0) ;
        }
    }


    public int getSize()
    {
        return m_iNumEntries ;
    }


    public ChartDescriptor getEntryAt(int index)
    {
        ChartDescriptor desc = null ;

        if ((index >= 0) && (index < m_iNumEntries))
        {
            desc = m_entries[index] ;
        }
        else
            throw new IllegalArgumentException() ;

        return desc ;
    }


    public void addChartModelListener(ChartModelListener listener)
    {
        if (!m_vListeners.contains(listener))
        {
             m_vListeners.addElement(listener) ;
        }
    }


    public void removeChartModelListener(ChartModelListener listener)
    {
        m_vListeners.removeElement(listener) ;
    }


    public void reindexData()
    {
        for (int i=0; i<m_iNumEntries; i++)
        {
            m_entries[i].sourceColumn = findKeyColumn(m_entries[i].source) ;
            m_entries[i].targetColumn = findKeyColumn(m_entries[i].target) ;

            if (m_entries[i].sourceColumn == -1)
            {
                addKey(ERROR_KEY) ;
                m_entries[i].sourceColumn = findKeyColumn(ERROR_KEY) ;
            }

            if (m_entries[i].targetColumn == -1)
            {
                addKey(ERROR_KEY) ;
                m_entries[i].targetColumn = findKeyColumn(ERROR_KEY) ;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void addBranchIndex(String strBranch, ChartDescriptor desc)
    {
        Vector vEntries = (Vector) m_htBranchIndex.get(strBranch) ;
        if (vEntries == null)
            vEntries = new Vector() ;

        vEntries.addElement(desc) ;
        m_htBranchIndex.put(strBranch, vEntries) ;
    }


    protected Enumeration getDescForBranch(String strBranch)
    {
        Enumeration enumRC = null ;

        Vector vEntries = (Vector) m_htBranchIndex.get(strBranch) ;
        if (vEntries != null)
        {
            enumRC = vEntries.elements() ;
        }

        return enumRC ;
    }


    // Find the string which a key is aliased to.
    // If the key is in m_trackableKeys, it is returned unchanged.
    // If the key is listed as an alias, its aliased entry in m_trackableKeys
    // is returned.
    // If neither, null is returned.
    protected String findKey(String objTrackableKey)
    {
       objTrackableKey = massageKey(objTrackableKey) ;

        String objRC = null ;

        // First check real keys
        for (int i=0; i<m_iNumKeys; i++)
        {
            if (objTrackableKey.equals(m_trackableKeys[i]))
            {
                objRC = m_trackableKeys[i] ;
                break ;
            }
        }

        // Next, search alias list
        Enumeration enumerator = m_htAliases.keys() ;
        while ((objRC == null) && enumerator.hasMoreElements())
        {
            String objAliasesSrc = (String) enumerator.nextElement() ;
            Vector vElements = (Vector) m_htAliases.get(objAliasesSrc) ;
            Enumeration vEnum = vElements.elements() ;
            while ((objRC == null) && vEnum.hasMoreElements())
            {
                String obj = (String) vEnum.nextElement() ;
                if (obj.equals(objTrackableKey))
                {
                    objRC = objAliasesSrc ;
                }
            }
        }

        return objRC ;
    }


    // Find the column allocated to a key.
    // objTrackableKey is normalized with massageKey, but not searched for
    // as an alias.
    protected int findKeyColumn(String objTrackableKey)
    {
        objTrackableKey = massageKey(objTrackableKey) ;

        int iRC = -1 ;

        String key = findKey(objTrackableKey) ;
        if (key != null)
        {
            for (int i=0; i<m_iNumKeys; i++)
            {
                if (key.equals(m_trackableKeys[i]))
                {
                    iRC = i ;
                    break ;
                }
            }
        }
        else
        {
            System.out.println("Unable to find column for key: " + objTrackableKey) ;
        }

        return iRC ;
    }


    protected void fireKeyAdded(int position)
    {
        ChartModelListener listener ;
        Vector listeners = (Vector) m_vListeners.clone() ;

        Enumeration enumListeners = listeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            listener = (ChartModelListener) enumListeners.nextElement() ;
            listener.keyAdded(position) ;
        }
    }


    protected void fireKeyDeleted(int position)
    {
        ChartModelListener listener ;
        Vector listeners = (Vector) m_vListeners.clone() ;

        Enumeration enumListeners = listeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            listener = (ChartModelListener) enumListeners.nextElement() ;
            listener.keyDeleted(position) ;
        }
    }


    protected void fireKeyMoved(int oldPosition, int newPosition)
    {
        ChartModelListener listener ;
        Vector listeners = (Vector) m_vListeners.clone() ;

        Enumeration enumListeners = listeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            listener = (ChartModelListener) enumListeners.nextElement() ;
            listener.keyMoved(oldPosition, newPosition) ;
        }
    }


    protected void fireEntryAdded(int startPosition, int endPosition)
    {
        ChartModelListener listener ;
        Vector listeners = (Vector) m_vListeners.clone() ;

        Enumeration enumListeners = listeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            listener = (ChartModelListener) enumListeners.nextElement() ;
            listener.entryAdded(startPosition, endPosition) ;
        }
    }


    protected void fireEntryDeleted(int startPosition, int endPosition)
    {
        ChartModelListener listener ;
        Vector listeners = (Vector) m_vListeners.clone() ;

        Enumeration enumListeners = listeners.elements() ;
        while (enumListeners.hasMoreElements())
        {
            listener = (ChartModelListener) enumListeners.nextElement() ;
            listener.entryDeleted(startPosition, endPosition) ;
        }
    }


    // Normalize a key -- Replace :0 suffix with :5060.
    protected String massageKey(String strKey)
    {
        String strRC = strKey ;

        if (strRC.endsWith(":0"))
        {
            strRC = strRC.substring(0, strRC.length()-2) ;
            strRC += ":5060" ;
        }

        return strRC ;
    }
}
