package com.pingtel.sipviewer;

import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.List;

import javax.swing.BorderFactory;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.filechooser.FileNameExtensionFilter;

import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

public class SipViewerMetaData
{

    // input is the root container of the input file, it contains individual
    // XML elements that are SIP messages
    public static void setSipViewerMetaData(ChartHeader m_header, SIPChartModel m_model,
            SIPViewerFrame m_frame, JScrollPane m_scrollPane, JScrollPane m_scrollPaneSecond)
    {

        // first we see if sipviewer_meta data is embedded in the XML file
        List sipviewer_meta = SipBranchData.nodeContainer.getChildren("sipviewer_meta");

        // if we have an entry in the list then we got the data
        if (sipviewer_meta.size() != 0)
        {
            Element xmlNode;

            // get the JDOM Element object from the data, we use
            // magic number 0 since sipviewer entry in the xml file
            // is put there by sipviewer itself and we know that we will
            // only support one instance of the metadata (multiple instances
            // do not make sense, unless they are part of some future feature)
            xmlNode = (Element) sipviewer_meta.get(0);

            // set the column locations
            setKeyLocations(xmlNode.getChild("locations"), m_header);

            // set the background colors
            setBackgroundColors(xmlNode.getChild("colors"), m_model);

            if (xmlNode.getChild("display_locations") != null)
            {
                setDisplayLocations(xmlNode.getChild("display_locations"), m_model);
            }
            
            if (xmlNode.getChild("usage_counts") != null)
            {
                setUsageCounts(xmlNode.getChild("usage_counts"), m_model);
            }

            // set the view mode single/split
            setViewMode(xmlNode.getChildText("mode"), m_frame);

            // set the scroll locations for each pane
            setScrollLocations(xmlNode.getChild("scroll_locations"), m_scrollPane,
                    m_scrollPaneSecond);
        }
    }

    // sets location of each column, locations are extracted from the input XML
    // file
    private static void setKeyLocations(Element locations, ChartHeader m_header)
    {
        // used to store each location value
        List elementList = locations.getChildren("location");

        Element location;
        int count = elementList.size();
        for (int i = 0; i < count; i++)
        {
            // get the location element and then set the key position
            location = (Element) elementList.get(i);
            m_header.setKeyPosition(i, Double.valueOf(location.getText()));
        }
    }

    // sets background colors for each label/arrow combination, colors are
    // extracted from the input XML file
    private static void setBackgroundColors(Element colors, SIPChartModel m_model)
    {
        // used to hold all the background colors
        List elementList = colors.getChildren("color");

        Element color;
        int count = elementList.size();
        for (int i = 0; i < count; i++)
        {
            // get the color
            color = (Element) elementList.get(i);

            // lets get the background color stored in the file
            Color tmpColor = new Color(Integer.valueOf(color.getText()));

            // if the RGB values are same as the black color make sure to
            // explicitly set the background to the Color.BLACK since this is
            // laster used when drawing to determine if the background should
            // be painted at all, if Black is the color then background is not
            // painted and the column lines are not obscured by block
            // background, preserving
            // the original sipviewer look
            if (tmpColor.getRGB() == Color.BLACK.getRGB())
            {
                // this is the black color so set it to BLACK
                m_model.getEntryAt(i).backgroundColor = Color.BLACK;
            }
            else
            {
                // set the background color of the appropriate message
                m_model.getEntryAt(i).backgroundColor = tmpColor;
            }
        }
    }

    // sets the display index for each message, index of negative value
    // translates to invisible
    private static void setDisplayLocations(Element display_locations, SIPChartModel m_model)
    {
        // used to hold all the display locations
        List elementList = display_locations.getChildren("display_location");

        Element display_location;
        int count = elementList.size();

        for (int i = 0; i < count; i++)
        {
            // get the displayIndex
            display_location = (Element) elementList.get(i);

            // set the display index for this particular message
            m_model.getEntryAt(i).displayIndex = Integer.valueOf(display_location.getText());
        }
    }
    
    // sets the usage counts for all the columns
    private static void setUsageCounts(Element usage_counts, SIPChartModel m_model)
    {
        // used to hold all the usage counts
        List elementList = usage_counts.getChildren("usage_count");

        Element usage_count;
        int count = elementList.size();

        for (int x = 0; x < m_model.m_iNumKeys; x++)
        {            
            // get the actual count
            usage_count = (Element) elementList.get(x);

            // set the display index for this particular message
            m_model.m_keyUsage[x] = Integer.valueOf(usage_count.getText());
        }
    }

    // puts sipviewer in a single or split screen mode
    private static void setViewMode(String mode, SIPViewerFrame m_frame)
    {
        // if screen is in the split mode then mark the second pane as visible
        if (mode.equalsIgnoreCase("split"))
        {
            m_frame.setSecondPaneVisibility(true);
        }
    }

    // sets the scroll bar locations for both top and bottom panes
    private static void setScrollLocations(Element scrollLocations, JScrollPane m_scrollPane,
            JScrollPane m_scrollPaneSecond)
    {
        // get the bar relative location
        List elementList = scrollLocations.getChildren("scroll_location");

        Element scroll_location;
        int count = elementList.size();
        int maxBarSpan = 0;
        for (int i = 0; i < count; i++)
        {
            // get the scroll location
            scroll_location = (Element) elementList.get(i);

            if (i == SIPViewerFrame.topPaneID)
            {
                // we are dealing with the top panel (id 0), get the maximum
                // value for the scroll bar
                maxBarSpan = m_scrollPane.getVerticalScrollBar().getMaximum();

                // get the real scroll location by multiplying the maximum bar
                // range with the relative location, and set the current
                // location
                // of the bar to this location
                m_scrollPane.getVerticalScrollBar().setValue(
                        (int) (maxBarSpan * Double.valueOf(scroll_location.getText())));
            }
            else
            {
                // same as above but done for the second pane, even if its
                // invisible
                maxBarSpan = m_scrollPaneSecond.getVerticalScrollBar().getMaximum();
                m_scrollPaneSecond.getVerticalScrollBar().setValue(
                        (int) (maxBarSpan * Double.valueOf(scroll_location.getText())));
            }
        }
    }

    public static void saveSipViewerMetaData(SIPViewerFrame frameRef, String openedFileName,
            ChartHeader m_header, SIPChartModel m_model, SIPViewerFrame m_frame,
            JScrollPane m_scrollPane, JScrollPane m_scrollPaneSecond)
    {

        JPanel accesoryPanel = new JPanel(new GridLayout(2, 1));
        accesoryPanel.setBorder(BorderFactory.createTitledBorder("Save options"));

        JCheckBox checkbox = new JCheckBox("Omit Invisible Dialogs", null);
        checkbox.setSelected(false);

        SaveOptionCheckBox checkBoxListener = new SaveOptionCheckBox(checkbox);

        checkbox.addActionListener(checkBoxListener);
        accesoryPanel.add(checkbox);

        // Create a file chooser
        JFileChooser fc = new JFileChooser();
        FileNameExtensionFilter filter = new FileNameExtensionFilter("XML Log", "xml");
        fc.setFileFilter(filter);
        fc.setAccessory(accesoryPanel);
        fc.setSelectedFile(new File(openedFileName));

        // Show the "save" file dialog
        int returnVal = fc.showSaveDialog(frameRef);

        // if user pressed save lets do the input checking
        if (returnVal == JFileChooser.APPROVE_OPTION)
        {
            File fileName = fc.getSelectedFile();

            // if user actually entered something process the input
            if ((fileName.getName() != null) && (fileName.getName().length() > 0))
            {
                // file must end with a png extension, make user type it in
                if (!fileName.getName().toLowerCase().endsWith(".xml"))
                {
                    JOptionPane.showMessageDialog(null, "Error: file name must end with \".xml\".",
                            "Wrong file name", 1);
                }
                else
                {
                    // used to show the save file selector
                    File saveFile = fc.getSelectedFile();

                    // used to format the XML data in a more human readable
                    // format
                    Format newFormat = Format.getPrettyFormat();

                    // getting the JDOM outputter and setting the XML format
                    XMLOutputter outputter = new XMLOutputter();
                    outputter.setFormat(newFormat);

                    // if the save file name is the same as the open file name
                    // then we are doing an overwrite
                    if (saveFile.getAbsoluteFile().toString().equalsIgnoreCase(openedFileName))
                    {
                        // since we are overwriting all we need to do is append
                        // the meta data settings to the existing file
                        try
                        {

                            // if the meta data was in the opened file lets get
                            // rid of it since we'll be rewriting it with new
                            // information
                            SipBranchData.nodeContainer.removeChild("sipviewer_meta");
                            SipBranchData.nodeContainer.addContent(constructMetaData(m_header,
                                    m_model, m_frame, m_scrollPane, m_scrollPaneSecond));

                            FileWriter writer = new FileWriter(saveFile.getAbsoluteFile());
                            outputter.output(SipBranchData.traceDoc, writer);
                            writer.close();

                        } catch (IOException e)
                        {
                        }
                    }
                    else
                    {
                        // if the meta data was in the opened file lets get rid
                        // of it since we'll be rewriting it with new
                        // information
                        SipBranchData.nodeContainer.removeChild("sipviewer_meta");

                        // file names are different this is a brand new file,
                        // create a new file then and store the meta data
                        SipBranchData.nodeContainer.addContent(constructMetaData(m_header, m_model,
                                m_frame, m_scrollPane, m_scrollPaneSecond));

                        try
                        {
                            FileWriter writer = new FileWriter(saveFile.getAbsoluteFile());
                            outputter.output(SipBranchData.traceDoc, writer);
                            writer.close();
                        } catch (IOException e)
                        {
                        }
                    }
                }
            }
        }
        else
        {
            // user canceled the request do nothing
        }
    }

    // constructs the sipviewer_meta data component of the XML log file
    public static Element constructMetaData(ChartHeader m_header, SIPChartModel m_model,
            SIPViewerFrame m_frame, JScrollPane m_scrollPane, JScrollPane m_scrollPaneSecond)
    {
        // creating top level elements for constructing the sipviewer_meta XML
        // section
        Element meta = new Element("sipviewer_meta");
        Element locations = new Element("locations");
        Element usage_counts = new Element("usage_counts");
        Element colors = new Element("colors");
        Element display_locations = new Element("display_locations");
        Element scroll_locations = new Element("scroll_locations");

        // getting current column locations
        double keyPositions[] = m_header.getKeyPositions();

        // storing the column locations in XML format
        for (int x = 0; x < m_model.m_iNumKeys; x++)
        {
            // if user wants to delete invisible messages then
            // any columns that have not usage against them,
            // basically that have no messages "connecting"
            // to them should not be saved
            if (SaveOptionCheckBox.getCheckStatus())
            {
                // only save column location if it has
                // visible messages against it
                if ((m_model.m_keyUsage[x] != 0))
                {
                    Element location = new Element("location");
                    location.setText(Double.toString(keyPositions[x]));

                    // storing the locations under the "locations" XML tag
                    locations.addContent(location);
                    
                    // now lets store the usage count for the columns, basically
                    // how many times are they really a target or sounrce of a
                    // message
                    Element usage_count = new Element("usage_count");
                    usage_count.setText(Integer.toString(m_model.m_keyUsage[x]));

                    // storing the usage_count under the "locations" XML tag
                    usage_counts.addContent(usage_count);
                }
            }
            else
            {
                // user does not want to remove the invisible
                // messages so make sure to save all column locations
                // regardless of their key use
                Element location = new Element("location");
                location.setText(Double.toString(keyPositions[x]));

                // storing the locations under the "locations" XML tag
                locations.addContent(location);
                
                // now lets store the usage count for the columns
                Element usage_count = new Element("usage_count");
                usage_count.setText(Integer.toString(m_model.m_keyUsage[x]));

                // storing the usage_count under the "locations" XML tag
                usage_counts.addContent(usage_count);
            }
        }

        // adding locations to the meta data component
        meta.getChildren().add(locations);
        
        // adding usage counts to the meta data component
        meta.getChildren().add(usage_counts);

        // if the delete invisible message check box is checked
        // then we have to remove the messages from the XML structure
        if (SaveOptionCheckBox.getCheckStatus())
        {
            // get the list of branchNode elements
            List elementList = SipBranchData.nodeContainer.getChildren("branchNode");

            // getting number of elements
            int count = elementList.size();

            // need to adjust since we are going to be
            // moving backwards
            count--;

            // loop through all the elements backwards since once
            // we start deleting the indexes shift, going backwards
            // the lower indexes are still accurate
            for (int i = count; i >= 0; i--)
            {
                // if the message is invisible then remove it
                if (m_model.getEntryAt(i).displayIndex < 0)
                {
                    elementList.remove(i);
                }
            }
        }

        // storing the message locations and color in XML format
        for (int x = 0; x < m_model.getSize(); x++)
        {
            Element color = new Element("color");
            Element display_location = new Element("display_location");

            // if the box is checked to delete all hidden messages
            // then we don't want to store display_locations of
            // those messages, basically if displayIndex is < 0 we
            // skip it
            if (SaveOptionCheckBox.getCheckStatus())
            {
                // if displayIndex is of a visible message lets
                // store its index in the XML structure, else
                // we skip the message
                if (m_model.getEntryAt(x).displayIndex >= 0)
                {
                    color.setText(Integer.toString(m_model.getEntryAt(x).backgroundColor.getRGB()));

                    // storing the colors under the "colors" XML tag
                    colors.addContent(color);

                    display_location.setText(Integer.toString(m_model.getEntryAt(x).displayIndex));

                    // storing the message location under "display_location" XML
                    // tag
                    display_locations.addContent(display_location);
                }
            }
            else
            {
                color.setText(Integer.toString(m_model.getEntryAt(x).backgroundColor.getRGB()));

                // storing the colors under the "colors" XML tag
                colors.addContent(color);

                // user wants to keep all the messages as they are
                display_location.setText(Integer.toString(m_model.getEntryAt(x).displayIndex));

                // storing the message location under "display_location" XML tag
                display_locations.addContent(display_location);
            }
        }

        // adding colors to the meta data component
        meta.getChildren().add(colors);

        // adding display_locations to the meta data component
        meta.getChildren().add(display_locations);

        // mode is a single entry so don't need a top XML container for it
        Element mode = new Element("mode");

        if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
        {
            // if the bottom panel is visible then we are in the split mode
            mode.setText("split");
        }
        else
        {
            // if the bottom panel is not visible then we are in the single mode
            mode.setText("single");
        }

        // adding mode to the meta data component
        meta.getChildren().add(mode);

        // creating and storing the relative scroll position of the top and
        // bottom panels
        Element scroll_location1 = new Element("scroll_location");

        scroll_location1.setText(Double.toString((double) m_scrollPane.getVerticalScrollBar()
                .getValue()
                / (double) m_scrollPane.getVerticalScrollBar().getMaximum()));
        scroll_locations.addContent(scroll_location1);

        Element scroll_location2 = new Element("scroll_location");

        scroll_location2.setText(Double.toString((double) m_scrollPaneSecond.getVerticalScrollBar()
                .getValue()
                / (double) m_scrollPaneSecond.getVerticalScrollBar().getMaximum()));
        scroll_locations.addContent(scroll_location2);

        // adding the scroll positions to the meta data component
        meta.getChildren().add(scroll_locations);

        return meta;
    }

    // this dialog shows the colors available for setting the sip dialog
    // backgrounds
    static public class SaveOptionCheckBox extends JDialog implements ActionListener
    {
        static JCheckBox checkboxRef;

        // class constructor
        public SaveOptionCheckBox(JCheckBox checkbox) {
            checkboxRef = checkbox;
            checkboxRef.setSelected(false);
        }

        // handles the events, if user clicks on one of the buttons the
        // selectedColor is assigned its value and the color chooser
        // dialog disappears
        public void actionPerformed(ActionEvent e)
        {

            // System.out.print(checkboxRef.isSelected());

        }

        static public boolean getCheckStatus()
        {
            return checkboxRef.isSelected();
        }

    }
}
