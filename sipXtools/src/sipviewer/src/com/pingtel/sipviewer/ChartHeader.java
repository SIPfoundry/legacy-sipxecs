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
    protected String[]      m_keys ;
    protected int           m_iNumKeys ;
    protected SIPInfoPanel  m_infoPanel ;
    protected int           m_iMouseOver ;


//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    public ChartHeader(SIPChartModel model, SIPInfoPanel infoPanel)
    {
        m_model = model ;
        m_infoPanel = infoPanel ;
        m_iMouseOver = -1 ;

        m_model.addChartModelListener(new icChartModelListener());
        addMouseMotionListener(new icMouseMotionListener()) ;
        addMouseListener(new icMouseListener(this)) ;


        refreshKeyCache() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void paint(Graphics g)
    {
        Dimension dimSize = getSize() ;

        if (dimSize.width > 0)
        {
            g.setColor(Color.black);
            g.fillRect(0, 0, dimSize.width, dimSize.height);
        }

        if ((dimSize.width > 0) && (m_iNumKeys > 0))
        {
            int iOffset = dimSize.width / m_iNumKeys ;
            for (int i=0; i<m_iNumKeys; i++)
            {
                Rectangle rectTextArea = new Rectangle(
                        iOffset*i,
                        0,
                        iOffset,
                        dimSize.height) ;


                int xOffset = GUIUtils.calcXOffset(m_keys[i], g, rectTextArea, GUIUtils.ALIGN_CENTER) ;
                int yOffset ;
                if ((i+1) % 2 == 0)
                {
                    g.setColor(Color.yellow);
                    g.drawLine((iOffset*i) + iOffset / 2,
                        dimSize.height/2,
                        (iOffset*i) + iOffset / 2,
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
            iIndex = point.x / (dimSize.width / m_iNumKeys) ;
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


    protected void setMouseOver(int index)
    {
        int iOldIndex = m_iMouseOver ;
        m_iMouseOver = index ;
        if (iOldIndex != m_iMouseOver)
        {
            m_infoPanel.clear() ;
            m_infoPanel.setMessage(getAliasReport(m_iMouseOver));
            repaint() ;
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
            int index = pointToIndex(e.getPoint()) ;

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

        }

        public void mouseReleased(MouseEvent e)
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
}
