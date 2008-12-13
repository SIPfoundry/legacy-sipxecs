package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;

public class SIPChartControlPanel extends Container implements ActionListener
{
    protected SIPChartModel m_model ;

    protected JLabel        m_lblKey ;
    protected JTextField    m_tfKey ;

    protected JLabel        m_lblAlias ;
    protected JTextField    m_tfAlias ;

    protected JButton       m_btnAdd ;

    public SIPChartControlPanel(SIPChartModel model)
    {
        m_model = model ;

        createComponents() ;
        layoutComponents() ;
    }

    public void actionPerformed(ActionEvent event)
    {
        System.out.println("Action Performed") ;
        String strKey = m_tfKey.getText() ;
        String strAlias = m_tfAlias.getText() ;

        m_model.addKeyAlias(strKey, strAlias);
    }

    protected void createComponents()
    {
        m_lblKey = new JLabel("Key:") ;
        m_tfKey = new JTextField() ;

        m_lblAlias = new JLabel("Alias:") ;
        m_tfAlias = new JTextField() ;


        m_btnAdd = new JButton("Add") ;
    }

    protected void layoutComponents()
    {
        setLayout(new GridBagLayout()) ;
        GridBagConstraints gbc = new GridBagConstraints() ;

        gbc.weighty = 0.0 ;
        gbc.insets = new Insets(4,4,4,4) ;

        gbc.weightx = 0.0 ;
        gbc.fill = GridBagConstraints.NONE ;
        add(m_lblKey, gbc) ;

        gbc.weightx = 1.0 ;
        gbc.gridwidth = GridBagConstraints.REMAINDER ;
        gbc.fill = GridBagConstraints.HORIZONTAL ;
        add(m_tfKey, gbc) ;

        gbc.weightx = 0.0 ;
        gbc.gridwidth = 1 ;
        gbc.fill = GridBagConstraints.NONE ;
        add(m_lblAlias, gbc) ;

        gbc.weightx = 1.0 ;
        gbc.fill = GridBagConstraints.HORIZONTAL ;
        add(m_tfAlias, gbc) ;

        gbc.weightx = 0.0 ;

        gbc.fill = GridBagConstraints.NONE ;
        add(m_btnAdd, gbc) ;

        m_btnAdd
        .addActionListener(this);
    }
}
