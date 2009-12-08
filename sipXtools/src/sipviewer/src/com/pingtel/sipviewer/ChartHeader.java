package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;


public class ChartHeader extends Component
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected static final int DEFAULT_MAX_WIDTH    = 200 ;
    protected static final int DEFAULT_MAX_HEIGHT   = 100 ;
    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected SIPChartModel m_model ;
    protected ChartBody     m_body ;
    protected ChartBody     m_bodySecond ;
    protected String[]      m_keys ;
    protected double []     m_key_positions ;
    protected int           m_iNumKeys ;
    protected SIPInfoPanel  m_infoPanel ;
    protected int           m_iMouseOver ;
    protected int           m_movingKeyIndex ;


//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    public ChartHeader(SIPChartModel model, ChartBody body, ChartBody bodySecond, SIPInfoPanel infoPanel)
    {
        m_model = model ;
        m_body = body;
        m_bodySecond = bodySecond;
        m_infoPanel = infoPanel ;
        m_iMouseOver = -1 ;
        m_movingKeyIndex = -1;

        m_model.addChartModelListener(new icChartModelListener());
        addMouseMotionListener(new icMouseMotionListener()) ;
        addMouseListener(new icMouseListener(this)) ;

        // initialize the local key position array to the one initialized in
        // the SIPChartModel class, we do it this way so we can easily share
        // the position X coordinate for the columns with the ChartBody
        m_key_positions = m_model.getKeyPositions();
        
        refreshKeyCache() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void paint(Graphics g)
    {
        Dimension dimSize = getSize() ;

        // get the dimension of the window and set the background to black
        if (dimSize.width > 0)
        {
            g.setColor(Color.black);
            g.fillRect(0, 0, dimSize.width, dimSize.height);
        }

        // determine the spacing between the different columns, this depends
        // on the number of keys we are dealing with, keys==columns==participants in call
        if ((dimSize.width > 0) && (m_iNumKeys > 0))
        {
            // offset is the distance between each column
            int iOffset = dimSize.width / m_iNumKeys ;
            for (int i=0; i<m_iNumKeys; i++)
            {
                Rectangle rectTextArea = new Rectangle(
                        (int) (m_key_positions[i] * dimSize.width) - (iOffset / 2),
                        0,
                        iOffset,
                        dimSize.height) ;


                int xOffset = GUIUtils.calcXOffset(m_keys[i], g, rectTextArea, GUIUtils.ALIGN_CENTER) ;
                int yOffset ;
                
                // remainder is used to alternate between having the column text higher or lower and
                // weather to draw the small line if text is higher
                if ((i+1) % 2 == 0)
                {
                    // draw the short line snippet of the vertical lines in the header
                    // area only, the remaining part of the line is drawn in ChartBody paint method
                    g.setColor(Color.yellow);
                    g.drawLine((int) (m_key_positions[i] * dimSize.width),
                        dimSize.height/2,
                        (int) (m_key_positions[i] * dimSize.width),
                        dimSize.height) ;

                    yOffset = GUIUtils.calcYOffset(m_keys[i], g, rectTextArea, GUIUtils.ALIGN_NORTH) ;
                }
                else
                {
                    yOffset = GUIUtils.calcYOffset(m_keys[i], g, rectTextArea, GUIUtils.ALIGN_SOUTH) ;
                }


                if (m_iMouseOver == i)
                    g.setColor(Color.red);
                else
                    g.setColor(Color.yellow);
                
                // draw the key text on top of column
                g.drawString(m_keys[i], xOffset, yOffset) ;
            }
        }
    }


    public Dimension getPreferredSize()
    {
        return getMinimumSize() ;
    }


    public Dimension getMinimumSize()
    {
        int width = DEFAULT_MAX_WIDTH ;
        int height = DEFAULT_MAX_HEIGHT ;

        Graphics g = getGraphics() ;
        if (g != null)
        {
            Font f = getFont() ;
            if (f != null)
            {
                FontMetrics fm =  g.getFontMetrics(f) ;
                height = fm.getHeight() * 2 ;
            }
        }
        return new Dimension(width, height) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void refreshKeyCache()
    {
        m_keys = m_model.getKeys() ;
        m_iNumKeys = m_model.getNumKeys() ;
    }


    protected int pointToIndex(Point point)
    {
        Dimension dimSize = getSize() ;
        int       iIndex = -1 ;

        if ((dimSize.width > 0) && (m_iNumKeys > 0))
        {           
            // lets convert the point to a % from the beginning of
            // the frame, so we can compare it with column locations
            // which are also stored in % format  
            double convertedPoint = (double) point.x / (double) dimSize.width;                        
            
            for (int i=0; i<m_iNumKeys; i++)
            {
                // if the point is within %1 distance of a line (on either side) then
                // we found the column
                if ((convertedPoint < (m_key_positions[i] + 0.01)) &&
                    (convertedPoint > (m_key_positions[i] - 0.01)))   
                {
                    iIndex = i;
                    
                    // we got the index lets exit loop
                    break;
                }                
            }
        }
        return iIndex ;
    }

    protected int indexToXOffset(int index)
    {
        Dimension dimSize = getSize() ;
        int iWidth = (dimSize.width / m_iNumKeys) ;

        return  (iWidth * index) + iWidth / 2 ;
    }

    protected String getAliasReport(int iIndex)
    {
        String strRC = null ;

        if ((iIndex >= 0) && (iIndex < m_iNumKeys))
        {
            String strKey = m_keys[iIndex] ;
            Vector vAliases = m_model.getKeyAliases(strKey) ;

            strRC = "Key: " + strKey + "\r\n" ;
            if (vAliases != null)
            {
                for (int i=0; i<vAliases.size(); i++)
                {
                    String strAlias = (String) vAliases.elementAt(i) ;
                    strRC += "Aliases: " ;
                    strRC += strAlias ;
                    strRC += "\r\n" ;
                }
            }
        }

        return strRC ;
    }

    // invoked when mouse pointer moves over key text on top
    // of the column
    protected void setMouseOver(int index)
    {
        int iOldIndex = m_iMouseOver ;
        m_iMouseOver = index ;
        
        // only execute if there was actuall change in index from
        // the previous instance and if the index is not -1
        if ((iOldIndex != m_iMouseOver) && (index != -1))
        {
        	// clear the panel first
            m_infoPanel.clear() ;
                        
            // loop over the keys and find all the keys that have the
            // exact same position, if there is none then only the
            // current key will be displayed in the text panel, however
            // if user nested columns then we want to display all the
            // keys that are part of the one unified column            
            for (int i=0; i < m_iNumKeys; i++)
            {            
            	if (m_key_positions[i] == m_key_positions[index])
            	{
            		// positions match so lets append this key
            		// to the text panel
            		m_infoPanel.appendMessage(getAliasReport(i));
            	}
            }
            
            repaint() ;
        }
    }

    // calculate the offset for each key column from the start of the
    // scroll window
    protected void calculateKeyOffset()
    {   
    	// if the window size is less then/or 0 that means that
    	// sipviewer is invoked from command line with the import
    	// file as the parameter so we use the default size of 800
    	// as the window width
    	int windowSize  = 800;
    	
        Dimension dimSize = getSize() ;
        
        if (dimSize.width > 0)
        	windowSize = dimSize.width;

        // get the dimension of the window
        if ((windowSize > 0) && (m_iNumKeys != 0))
        {
            // delta between each column
            int delta = windowSize / m_iNumKeys ;
            
            // lets loop over all the keys and calculate their distance
            // from the beginning of the window 
            for (int i=0; i<m_iNumKeys; i++)
            {
                // we don't want to start the first column all the
                // way of delta distance from the left side but
                // rather only half that distance, then all the other
                // columns will be spaced out by delta
                m_key_positions[i] = (delta*i) + delta / 2;
                
                // the position is absolute but we want the position to
                // be a relative % so that we can move the columns and yet
                // have their position to be relatively the same regardless
                // of the window size
                m_key_positions[i] = m_key_positions[i] / windowSize;                
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Nested Classes
////

    protected class icChartModelListener implements ChartModelListener
    {
        public void keyAdded(int position)
        {
            refreshKeyCache() ;
            
            // keys are added when a file is loaded or when its reloaded
            // every time a key is added we must recalculate the column
            // positions for each key
            calculateKeyOffset();
            
            repaint() ;
        }

        public void keyDeleted(int position)
        {
            refreshKeyCache() ;
            repaint() ;
        }


        public void keyMoved(int oldPosition, int newPosition)
        {
            refreshKeyCache() ;
            repaint() ;
        }


        public void entryAdded(int startPosition, int endPosition) { }
        public void entryDeleted(int startPosition, int endPosition) { }
    }


    public class icMouseMotionListener implements MouseMotionListener
    {
        public void mouseDragged(MouseEvent e)
        {
           Point point = e.getPoint();
            
           // m_movingKeyIndex is set when user clicks on the
           // screen and is close to a column
            if (m_movingKeyIndex != -1)
            {
                Dimension dimSize = getSize() ;

                double convertedPoint = (double) point.x / (double) dimSize.width;                    
            
                // keep setting the location of this column to the current location
                m_key_positions[m_movingKeyIndex] = convertedPoint;
                
                // repaint the header components and also body components
                repaint();
                m_body.repaint();
                
                // repaint the second pane only if its visible
                if (m_bodySecond.isVisible())
                	m_bodySecond.repaint();
            }
        }

        public void mouseMoved(MouseEvent e)
        {
            setMouseOver(pointToIndex(e.getPoint())) ;
        }
    }

    protected class icMouseListener implements MouseListener
    {
        protected Component m_parent ;


        public icMouseListener(Component parent)
        {
            m_parent = parent ;
        }


        public void mouseClicked(MouseEvent e)
        {
            Point point = e.getPoint();
            
            int index = pointToIndex(point) ;
            
            if ((index >= 0) && (index < m_keys.length))
            {          
                int xOffset = indexToXOffset(index) ;

                if (e.getPoint().x > xOffset)
                {
                    m_model.moveKeyRight(m_keys[index]);
                }
                else
                {
                    m_model.moveKeyLeft(m_keys[index]);
                }
            }
        }

        public void mousePressed(MouseEvent e)
        {
            Point point = e.getPoint();
            
            int index = pointToIndex(point) ;
            
            // find out which column the user clicked on
            if ((index >= 0) && (index < m_keys.length))
            {           
            	// set the column number to the one that
            	// user pressed the mouse button on
                m_movingKeyIndex = index;
            }        
        }

        public void mouseReleased(MouseEvent e)
        {
        	// only execute if user clicked on a column and
        	// is currently dragging a mouse
            if (m_movingKeyIndex != -1)
            {
                Point point = e.getPoint();
                Dimension dimSize = getSize();
            
                // lets get the current position converted to the ratio distance
                // from the beginning of the window
                double convertedPoint = (double) point.x / (double) dimSize.width;                        
            
                // lets loop through all the keys
                for (int i=0; i<m_iNumKeys; i++)
                {
                    // if the point is within %1 distance of a line (on either side)
                	// then we found the column, also make sure that the column we
                	// found is no the one that we are currently moving around
                    if ((m_iNumKeys != m_movingKeyIndex) && 
                        ((convertedPoint < (m_key_positions[i] + 0.01)) &&
                        (convertedPoint > (m_key_positions[i] - 0.01))))   
                    {
                        // user is releasing the mouse button and the column he is
                    	// moving is within 1% of another column so lets make them
                    	// both snap in into one column
                        m_key_positions[m_movingKeyIndex] = m_key_positions[i];
                 
                        // force repaint of the header and the body
                        repaint();
                        m_body.repaint();
                        
                        // repaint the second pane only if its visible
                        if (m_bodySecond.isVisible())
                        	m_bodySecond.repaint();
                    }                
                }
            }
            
            // no more tracking of this column lets reset
            m_movingKeyIndex = -1;
        }
        
        public void mouseDragged(MouseEvent e)
        {
          
        }

        public void mouseEntered(MouseEvent e)
        {

        }


        public void mouseExited(MouseEvent e)
        {
            setMouseOver(-1);
        }
    }
    
    public double [] getKeyPositions()
    {
    	return m_key_positions;
    }
    
    public void setKeyPosition(int index, double position)
    {
    	// keep setting the location of this column to the current location
    	m_key_positions[index] = position;
    }
}
