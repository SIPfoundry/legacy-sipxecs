package org.sipfoundry.callpilot;

import java.io.File;
import java.io.IOException;

import org.apache.commons.io.FileUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.Collect;
import org.sipfoundry.commons.freeswitch.Localization;
import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.sipxivr.GreetingType;
import org.sipfoundry.sipxivr.IvrConfiguration;
import org.sipfoundry.sipxivr.Mailbox;
import org.sipfoundry.sipxivr.RestfulRequest;
import org.sipfoundry.voicemail.ExtMailStore;
import org.sipfoundry.voicemail.Greeting;
import org.sipfoundry.voicemail.VmDialog;
import org.sipfoundry.voicemail.VoiceMail;

public class CpGrtAdminDialog {

    private VoiceMail m_vm;
    private Logger m_log;
    private Mailbox m_mailbox;
    
    public CpGrtAdminDialog(VoiceMail callPilot, Logger log) {
        m_vm = callPilot;
        m_log = log;
        m_mailbox = m_vm.getMailbox();
    }
        
    private File makeTempWavFile() {
        File wavFile = null;
        try {
            // Create in the deleted directory, so if somehow things fail, they'll get removed
            // as they age out.
            wavFile = File.createTempFile("temp_recording_", ".wav", 
                                          new File(m_mailbox.getDeletedDirectory()));
        } catch (IOException e) {
            throw new RuntimeException("Cannot create temp recording file", e);
        }
        return wavFile;
    }
    
    public void rcdSpnName() {
        
       File tempWav = null; 
       
       try {              
           PromptList prePl = null;
           File nameFile = m_mailbox.getRecordedNameFile();
           
           if(nameFile.exists()) {
              prePl = new PromptList(m_vm.getLoc());
              prePl.addPrompts(nameFile.getPath());
           }
           
           tempWav = makeTempWavFile();                 
           File recordingFile = recordDialog(prePl, tempWav, "record_name", "confirm_name");
    
           if (recordingFile == null) {
               FileUtils.deleteQuietly(tempWav);
               tempWav = null;
               return;
           }
    
           // Save the recording as the name          
           if (nameFile.exists()) {
               nameFile.delete();
           }
              
           recordingFile.renameTo(nameFile);  
           tempWav = null;
           ExtMailStore.SaveSpokenNameInFolder(m_mailbox, nameFile);     

       } finally {
           if(tempWav != null) {
               FileUtils.deleteQuietly(tempWav);
           }
       }
       
       return;
    }
    
    /**
     * Record a wav file with confirmation dialog.
     * 
     * @param recordFragment  To play before the recording
     * @param confirmMenuFragment  To play after the recording
     * @return the temporary wav file.  null if recording is to be tossed
     */
    private File recordDialog(PromptList prePl, File wavFile, String recordFragment, String confirmMenuFragment) {
        String wavPath = wavFile.getPath();
        Localization loc = m_vm.getLoc();
        String ident = m_mailbox.getUser().getUserName();
    
        boolean recordWav = true ;
        boolean playWav = false;
        
        if(prePl != null) {
            loc.play(prePl, "*#");
        }
        
        for(;;) {    
            if (recordWav) {
                // Record your {thingy} then press #
                loc.play(recordFragment, "*#");

                m_vm.recordMessage(wavPath);
                String digit = loc.getFreeSwitchEventSocketInterface().getDtmfDigit();
                if (digit == null) {
                    digit = "";
                }
                    
                m_log.debug("Retrieve::recordDialog record terminated collected ("+digit+")");
              
                if (digit.equals("*")) {
                 // "Canceled."
                    loc.play("cmd_canceled", "");
                    return null;
                }
                recordWav = false;
            }

            CpDialog vmd = new CpDialog(m_vm, confirmMenuFragment);
            if (playWav) {
                PromptList messagePl = new PromptList(m_vm.getLoc());
                messagePl.addPrompts(wavPath);
                vmd.setPrePromptList(messagePl);
            }
    
            String digit = vmd.collectDigit("12#");
    
            // bad entry, timeout, canceled
            if (digit == null) {
                return null;
            }
                
            m_log.info("Retrieve::recordDialog "+ ident +" options ("+digit+")");
    
            // "1" means play the recording
            if (digit.equals("1")) {
                m_log.info(String.format("Retrieve::recordDialog "+ident+" Playing back recording (%s)", wavPath));
                playWav = true ;
                continue ;
            }

            // "2" means "erase" and re-record
            if (digit.equals("2")) {
                recordWav = true ;
                continue ;
            }
            
            // "#" means accept the recording
            if (digit.equals("#")) {
                m_log.info(String.format("Retrieve::recordDialog "+ident+" accepted recording (%s)", wavPath));
                return wavFile;
            }
        }
    }    
    
    public void rcdAGreeting() {
        
        PromptList prePl = null;
        File tempWav = null; 
        
        CpDialog vmd = new CpDialog(m_vm, "record_which_greeting");

        String digit = vmd.collectDigit("12*");
        String path;  
        
        if(digit.equals("*")) {
            return;
        }
        
        boolean primary = digit.equals("1");
        Greeting greeting = new Greeting(m_vm);
        
        if(primary) {
           path = greeting.getGreetingPath(GreetingType.STANDARD);
        } else {
           path = greeting.getGreetingPath(GreetingType.OUT_OF_OFFICE); 
        }
      
        if(path != null) {
           prePl = new PromptList(m_vm.getLoc());
           prePl.addPrompts(path);
        }       
        
        vmd = new CpDialog(m_vm, "record_new_greeting");
        vmd.setPrePromptList(prePl);

        digit = vmd.collectDigit("1*");
        
        if(digit.equals("*")) {
            m_vm.getLoc().play("cmd_canceled", "");
            return;
        }
        
        try {             
            tempWav = makeTempWavFile();                 
            File recordingFile = recordDialog(prePl, tempWav, "greeting_record", "confirm_name");
     
            if (recordingFile == null) {
                FileUtils.deleteQuietly(tempWav);
                tempWav = null;
                return;
            }
            
            greeting.saveGreetingFile(primary ? GreetingType.STANDARD : GreetingType.OUT_OF_OFFICE, 
                                      recordingFile); 
            tempWav = null;       
        } finally {
            if(tempWav != null) {
                FileUtils.deleteQuietly(tempWav);
            }
        }
    }
    
    public void chooseAGreeting() {
        GreetingType type = GreetingType.NONE;
        
        CpDialog vmd = new CpDialog(m_vm, "select_greeting");
        
        String digit = vmd.collectDigit("12*");
        
        if(digit.equals("*")) {
            m_vm.getLoc().play("cmd_canceled", "");
            return;
        }
        
        if (digit.equals("1")) {
            type = GreetingType.STANDARD;
        } else {
            type = GreetingType.OUT_OF_OFFICE;
        }
    
        m_mailbox.getMailboxPreferences().getActiveGreeting().setGreetingType(type);
        m_mailbox.writeMailboxPreferences();
        
        if (digit.equals("1")) {
            m_vm.getLoc().play("use_primary_greeting", "");
        } else {
            m_vm.getLoc().play("use_alternate_greeting", "");
        }
    }
}
