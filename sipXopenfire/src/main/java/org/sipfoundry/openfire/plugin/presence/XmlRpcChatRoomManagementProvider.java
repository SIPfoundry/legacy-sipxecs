package org.sipfoundry.openfire.plugin.presence;

import java.util.Collection;
import java.util.Map;

import org.apache.log4j.Logger;

public class XmlRpcChatRoomManagementProvider extends XmlRpcProvider {
    
    private static Logger log = Logger.getLogger(XmlRpcChatRoomManagementProvider.class);
    public final static String SERVICE = "chatroom";
    public final static String SERVER = "chatRoomManagementServer";
   
    
    public Map createChatRoom(String subdomain, 
            String owner,
            String roomName,  
            String description, 
            String password, 
            String conferenceExtension) {
        
        try {
            boolean listRoom = true;
            boolean moderated = false;
            boolean membersOnly = false;
            boolean allowInvite = true;
            boolean publicRoom = false;
            boolean logConversations = false;
            boolean isPersistent = false;
            log.info(String.format("createChatRoom %s\n %s\n %s\n %s\n", 
                    subdomain,roomName,description,conferenceExtension));
            String ownerJid = this.appendDomain(owner);
            getPlugin().createChatRoom(subdomain,ownerJid,roomName, 
                    listRoom,moderated,membersOnly,allowInvite,publicRoom,logConversations,
                     isPersistent, password, description, conferenceExtension);
            return createSuccessMap();
        } catch (Exception ex) {
            log.error("Processing error " , ex);
            return createErrorMap(ErrorCode.CREATE_CHAT_ROOM,ex.getMessage());
        }
    }
    
    /**
     * Delete a room
     * 
     * @param subdomain -- server subdomain
     * @param roomName -- room name
     * @param userName -- userName
     */
    public Map removeChatRoom(String subdomain, String roomName) {
       try {
           getPlugin().removeChatRoom(subdomain, roomName);
           return createSuccessMap();
       } catch (Exception ex) {
           return createErrorMap(ErrorCode.REMOVE_CHAT_ROOM,ex.getMessage());
       }
       
    }
    
    /**
     * Get the chat room members.
     * 
     * @param subdomain -- server subdomain.
     * @param roomName -- room name.
     * @param userName -- userName
     *
     */
    public Map getMembers(String subdomain, String roomName) {
        try {
            Collection<String> members = getPlugin().getMembers(subdomain,roomName);
            Map retval = createSuccessMap();
            retval.put(ROOM_MEMBERS, members.toArray());
            return retval;
        } catch ( Exception ex) {
            return createErrorMap(ErrorCode.GET_CHAT_ROOM_MEMBERS,ex.getMessage());
             
        }
    }
    
    /**
     * Destroy a MUC service.
     */
    public Map destroyMultiUserChatService(String subdomain) {
        try {
            getPlugin().destroyMultiUserChatService(subdomain);
            return createSuccessMap();
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.DESTROY_MULTI_USER_CHAT_SERVICE,ex.getMessage());
        }
    }
    
    
    public Map getAttributes(String subdomain, String roomName) {
        try {
            Map attributes =  getPlugin().getMucRoomAttributes(subdomain,roomName);
            Map retval = createSuccessMap();
            retval.put("attributes", attributes);      
            return retval;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.GET_CHAT_ROOM_ATTRIBUTES, ex.getMessage());
        }
    }
    
    public Map setAttributes(String domain, String roomName, Map newAttributes ) {
        try {
            Map originalAttributes = getPlugin().getMucRoomAttributes(domain, roomName);
            originalAttributes.putAll(newAttributes);
            getPlugin().setMucRoomAttributes(domain,roomName, originalAttributes);
            Map retval = createSuccessMap();
            retval.put("attributes", originalAttributes);      
            return retval;
        } catch (Exception ex) {
            return createErrorMap(ErrorCode.SET_CHAT_ROOM_ATTRIBUTES, ex.getMessage());
        }
    }
    
    public Map getConferenceExtension(String domain, String roomName) {
        try {
            Map retval = createSuccessMap();
            String extension = getPlugin().getConferenceExtension(domain, roomName);
            retval.put(XmlRpcProvider.CONFERENCE_BRIDGE_EXTENSION, extension);
            return retval;
        }catch (Exception ex) {
            return createErrorMap(ErrorCode.GET_CHAT_ROOM_ATTRIBUTES, ex.getMessage());
        }
    }
    
    
    
}
