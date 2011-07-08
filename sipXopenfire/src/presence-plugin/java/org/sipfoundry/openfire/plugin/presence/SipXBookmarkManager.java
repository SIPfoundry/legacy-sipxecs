package org.sipfoundry.openfire.plugin.presence;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.ArrayList;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jivesoftware.database.DbConnectionManager;
import org.jivesoftware.openfire.container.PluginClassLoader;
import org.jivesoftware.openfire.container.PluginManager;

public class SipXBookmarkManager {
	private static SipXBookmarkManager instance;
	private PluginManager pluginManager;
	private Class<?> bookmarkClazz;
	private Class<?> bookmarkManagerClazz;
	private Constructor<?> bookmarkCtorNew;
	private Constructor<?> bookmarkCtorExist;
	private Class<?> typeClazz;
	private Method deleteBookmark;
	private Method setUsers;

	private static final String SELECT_BOOKMARK = "SELECT bookmarkID from ofBookmark where bookmarkname=?";

	private static Logger log = Logger.getLogger(SipXBookmarkManager.class);

	private SipXBookmarkManager(PluginManager manager) throws Exception {
		pluginManager = manager;
		PluginClassLoader clientcontrolLoader = pluginManager.getPluginClassloader(pluginManager.getPlugin("clientcontrol"));
		if(clientcontrolLoader == null) {
			log.info("Client Management Plugin is not installed");
			throw new Exception("Client Management Plugin is not installed");
		}
		bookmarkClazz = clientcontrolLoader.loadClass("org.jivesoftware.openfire.plugin.spark.Bookmark");
		typeClazz = bookmarkClazz.getClasses()[0];
		bookmarkManagerClazz = clientcontrolLoader.loadClass("org.jivesoftware.openfire.plugin.spark.BookmarkManager");
		bookmarkCtorNew = bookmarkClazz.getDeclaredConstructor(typeClazz, String.class, String.class);
		bookmarkCtorExist = bookmarkClazz.getDeclaredConstructor(long.class);
		deleteBookmark = bookmarkManagerClazz.getMethod("deleteBookmark", long.class);
		setUsers = bookmarkClazz.getMethod("setUsers", Collection.class);
	}

    public static synchronized void initialize(PluginManager pluginManager) {
        if (null == instance) {
		try {
			instance = new SipXBookmarkManager(pluginManager);
		} catch (Exception ex) {
			log.error("Cannot create manager ", ex);
		}
        }
    }

    public static SipXBookmarkManager getInstance() {
	if (instance == null) {
		log.error("Bookmark Manager is not initialized");
	}
	return instance;
    }

    public static boolean isInitialized() {
	return instance != null;
    }

    public void createMUCBookmark(String name, String value) {
	try {
		bookmarkCtorNew.newInstance(typeClazz.getEnumConstants()[1], name, value);
		log.info("BOOKMARK created: "+name+" with value: "+value);
	} catch (final Exception e) {
		// not pretty but we're catching several exceptions we can do little
		// about
		log.error("Error creating bookmark ", e);
	}
    }

	public Long getMUCBookmarkID(String roomName) {
		Connection con = null;
		PreparedStatement pstmt = null;
		ResultSet rs = null;
		Long id = null;
		try {
			con = DbConnectionManager.getConnection();
			pstmt = con.prepareStatement(SELECT_BOOKMARK);
			pstmt.setString(1, roomName);
			rs = pstmt.executeQuery();
			if (rs.next()) {
				id = rs.getLong(1);
			}
		} catch (final Exception e) {
			log.error("Cannot get bookmark id for: "+roomName, e);
		} finally {
			DbConnectionManager.closeConnection(rs, pstmt, con);
		}

		return id;

	}
	/**
	 * Deletes a bookmark being given the room name
	 * Make sure that SipXopenfire plugin class creates the singleton
	 * during initialization
	 * @param roomName
	 */
    public void deleteMUCBookmark(String roomName ) {
	try {
		long bookmarkId = getMUCBookmarkID(roomName);
		deleteBookmark.invoke(null, bookmarkId);
		log.info("Bookmark " + roomName + " deleted");
	} catch (final Exception e) {
		log.error("Error deleting bookmark "+roomName);
	}
    }

    public void setMUCBookmarkOwner(String roomName, String owner) {
	try {
		Long bookmarkID = getMUCBookmarkID(roomName);
		if (bookmarkID == null) {
			log.error("Bookmark "+roomName+" was not previously created");
			return;
		}
		Object bookmark = bookmarkCtorExist.newInstance(bookmarkID);
		Collection<String> ownerList = new ArrayList<String>();
		ownerList.add(owner);
		setUsers.invoke(bookmark, ownerList);

		log.info("Owner "+owner+ " set on bookmark "+roomName);
	} catch (final Exception e) {
		log.error("Error setting owner "+owner+" on bookmark "+roomName, e);
	}
    }

}
