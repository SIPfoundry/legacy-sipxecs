package org.sipfoundry.callpilot;

import org.sipfoundry.commons.freeswitch.PromptList;
import org.sipfoundry.voicemail.VoiceMail;

public class CpCmd {
    
    public enum Command {
        NONE,
        REVERT,      
        SKIPBACKWARD,
        PLAY,
        SKIPFORWARD,
        PREVIOUS,
        RECORD,
        NEXT,
        CALLSENDER,      
        CANCELED,
        MSGOPTIONS,
        REPLY,
        ENVELOP,
        FORWARD,
        REPLYALL,        
        COMPOSE,
        DELETE,
        SEND,
        LOGIN,       
        PWDCHANGE,
        TOOLS,
        GOTO,
        PERSONAL_ATTENDANT,
        REC_GREETING,
        CHOOSE_GREETING,
        REC_SPKNAME,
        TOGGLE_URGENCY
    }
    
    private PromptList m_prePl;
    private String m_immed;
    private String m_delay;
    private String m_help;
    
    private String m_midDelay;
    private String m_midHelp;
    private long m_duration;
    
    private VoiceMail m_vm;
    
    public CpCmd(VoiceMail callPilot) {    
        m_vm = callPilot;
    }
    
    public CpCmd(VoiceMail callPilot, String immed, String delay, String help) {
        m_immed = immed;
        m_delay = delay;
        m_help  = help;
        m_prePl = null;
        m_midDelay = null;
        m_midHelp  = null;
        m_duration = 0;
        
        m_vm = callPilot;
    }
    
    public void setPrePromptList(PromptList prePl) {
        m_prePl = prePl;
    }
    
    public void setPrePromptList(String prePrompt) {     
        m_prePl = m_vm.getLoc().getPromptList(prePrompt);
    }
    
    public Command CollectMsgCmd() {
        // at this point the user has already entered 7 ..
        CpDialog cpDialog = new CpDialog(m_vm, null, "msg_cmd_delay", "msg_cmd_help");
        
        for(;;) {
            String cmd = cpDialog.collectDigit("0123456789#*");
        
            // check for * and present help
            if(cmd.equals("*")) {
                cpDialog.setPrePromptList("msg_cmd_help");
                continue;
            }
                
            // command is cancelled
            if(cmd.equals("#")) {
                m_vm.getLoc().play("cmd_canceled", "");
                return Command.CANCELED;
            }
        
            if(cmd.equals("0")) {
                return Command.MSGOPTIONS;
            }     
            
            if(cmd.equals("1")) {
                return Command.REPLY;
            }
        
            if(cmd.equals("2")) {
                return Command.ENVELOP;    
            }
        
            if(cmd.equals("3")) {
                return Command.FORWARD;    
            }
        
            if(cmd.equals("4")) {
                return Command.REPLYALL;    
            }
        
            if(cmd.equals("5")) {
                return Command.COMPOSE;    
            }
        
            if(cmd.equals("6")) {
                return Command.DELETE;    
            }
            
            if(cmd.equals("9")) {
                return Command.SEND;    
            }
        
            // must be an invalid digit, play "invalid digit" and return no command
            if(cmd.length() > 0) {
                m_vm.playError("bad_cmd");  
                return null;
            }    
            
            return Command.CANCELED;
        }
    }

    public Command CollectMsgOptionsCmd() {
        // at this point user has entered 70

        CpDialog cpDialog = new CpDialog(m_vm, "msg_options");

        for(;;) { 
            String cmd = cpDialog.collectDigit("0123456789#*"); 

            if(cmd.equals("1")) {
                return Command.TOGGLE_URGENCY;
            }
            
            if(cmd.equals("*")) {
                cpDialog.setPrePromptList("msg_options");
                continue;
            }

            if(cmd.equals("#")) {
                m_vm.getLoc().play("cmd_canceled", "");
                return Command.CANCELED;
            }

            // must be an invalid digit, play "invalid digit" and return no command
            if(cmd.length() > 0) {
                m_vm.playError("bad_cmd");  
            }    
            
            return Command.CANCELED;
        } 
    }    
    
    public Command CollectMbxCmd() {
        // at this point the user has already entered 8 ..
        CpDialog cpDialog = new CpDialog(m_vm, null, "mbx_cmd_delay", "mbx_cmd_help");
        
        for(;;) {
            String cmd = cpDialog.collectDigit("0123456789#*");
        
            // check for * and present help
            if(cmd.equals("*")) {
                cpDialog.setPrePromptList("mbx_cmd_help");
                continue;
            }
                
            // command is cancelled
            if(cmd.equals("#")) {
                m_vm.getLoc().play("cmd_canceled", "");
                return Command.CANCELED;
            }
        
            if(cmd.equals("0")) {
                return CollectMbxOptionsCmd();
            }
        
            if(cmd.equals("1")) {
                return Command.LOGIN;
            }
        
            if(cmd.equals("2")) {
                return CollectGreetingsCmd();   
            }
        
            if(cmd.equals("3")) {
                m_vm.goodbye();
                return Command.CANCELED;    
            }
        
            if(cmd.equals("4")) {
                return Command.PWDCHANGE;    
            }
        
            if(cmd.equals("5")) {
                return Command.TOOLS;    
            }
        
            if(cmd.equals("6")) {
                return Command.GOTO;    
            }
        
            // must be an invalid digit, play "invalid digit" and return no command
            if(cmd.length() > 0) {
                m_vm.playError("bad_cmd");  
            }    
            
            return Command.CANCELED;
        }
    }
           
    private Command CollectGreetingsCmd() {
        // at this point the user has already entered 82 ..
        CpDialog cpDialog = new CpDialog(m_vm, "greetings_cmd_delay", "greetings_cmd_delay", "greetings_cmd_help");
        
        for(;;) {
            String cmd = cpDialog.collectDigit("0123456789#*");
        
            // check for * and present help
            if(cmd.equals("*")) {
                cpDialog.setPrePromptList("greetings_cmd_help");
                continue;
            }
            
            if(cmd.equals("1")) {
                return Command.REC_GREETING;
            }
            
            if(cmd.equals("2")) {
                return Command.CHOOSE_GREETING;
            }
            
            if(cmd.equals("9")) {
                return Command.REC_SPKNAME;
            }
                
            if(cmd.equals("#")) {
                m_vm.getLoc().play("cmd_canceled", "");
                return Command.CANCELED;
            }
            
            // must be an invalid digit, play "invalid digit" and return no command
            if(cmd.length() > 0) {
                m_vm.playError("bad_cmd");  
            } 
            
            return Command.CANCELED;
        }
    }    
    
    private Command CollectMbxOptionsCmd() {
        // at this point the user has already entered 80 ..
        CpDialog cpDialog = new CpDialog(m_vm, "mbxoptions_cmd_delay", "mbxoptions_cmd_delay", "mbxoptions_cmd_help");
        
        for(;;) {
            String cmd = cpDialog.collectDigit("0123456789#*");
        
            // check for * and present help
            if(cmd.equals("*")) {
                cpDialog.setPrePromptList("mbxoptions_cmd_help");
                continue;
            }
            
            if(cmd.equals("1")) {
                return Command.PERSONAL_ATTENDANT;
            }
                
            if(cmd.equals("#")) {
                m_vm.getLoc().play("cmd_canceled", "");
                return Command.CANCELED;
            }
            
            // must be an invalid digit, play "invalid digit" and return no command
            if(cmd.length() > 0) {
                m_vm.playError("bad_cmd");  
            } 
            
            return Command.CANCELED;
        }
    }    
    
    public void midMessagePlayBackPrompts(String delay, String help, long duration) {
        // used for message playback since delay and help prompt presentation
        // depends if the message plays to completion or not.
        m_midDelay = delay;
        m_midHelp  = help;
        m_duration = duration;
    }
    
    public Command CollectCmd() {
        CpDialog cpDialog = new CpDialog(m_vm, m_immed, m_delay, m_help);   
        cpDialog.setPrePromptList(m_prePl);
        Command cmdResult;
        long startPlayTime = System.currentTimeMillis();
        
        for(;;) {    
            
            String cmd = cpDialog.collectDigit("0123456789#*");
            
            if(cmd == null) {
                return null;
            }
            
            // got a digit. if digit received while in the middle of
            // playing a message then update the delay and help prompts
            if(m_midDelay != null) {
               if(System.currentTimeMillis() - startPlayTime < m_duration) {
                   m_delay = m_midDelay;
                   m_help  = m_midHelp;
                   m_midDelay = null;
               }
            }
            
               // m_startOfCmdTime = System.currentTimeMillis();
            
            if(cmd.equals("7")) {
                cmdResult = CollectMsgCmd();
                if(cmdResult == Command.CANCELED) {
                    cpDialog = new CpDialog(m_vm, null, m_delay, m_help); 
                    continue; 
                } else { 
                   return cmdResult;   
                }    
            }
            
            if(cmd.equals("8")) {
                cmdResult =  CollectMbxCmd();    
                if(cmdResult == Command.CANCELED) {
                    cpDialog = new CpDialog(m_vm, null, m_delay, m_help); 
                    continue; 
                } else { 
                   return cmdResult;   
                }    
            }
            
            if(cmd.equals("*")) {
                cpDialog = new CpDialog(m_vm, m_help, m_delay, m_help); 
                continue;
            }
   
            if(cmd.equals("1")) {
                return Command.SKIPBACKWARD;
            }
            
            if(cmd.equals("2")) {
                return Command.PLAY;
            }
            
            if(cmd.equals("3")) {
                return Command.SKIPFORWARD;
            }
            
            if(cmd.equals("4")) {
                return Command.PREVIOUS;
            }
            
            if(cmd.equals("5")) {
                return Command.RECORD;
            }
            
            if(cmd.equals("6")) {
                return Command.NEXT;
            }
            
            if(cmd.equals("9")) {
                return Command.CALLSENDER;
            }
            
            if(cmd.equals("0")) {
                return Command.REVERT;
            }
            
            return Command.NONE;                   
        }
    }
    
}
