/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
library sipxconfig;

import 'dart:html';
import 'dart:async';
import 'dart:collection';
import 'dart:convert';

/**
 * Common utilities for sipxecs-based UIs
 * Implementation Note: This could potentially be split into multiple files
 * but single file is a little easier to expose to plugins.  
 */

/**
 * Useful for scripts that load data from web services that wish to alternatively
 * load static data to render page for development purposes.  By using this utility
 * you can easily flip between getting data from web service and static files.
 * 
 * If page was commited to git with it left in static data mode, page will 
 * automatically use REST api when built into sipxconfig jar so pages can safely 
 * leave themselves in test mode.
 * 
 * Obviously this is only useful for GET requests.
 * 
 * Example:
 *   var api = new Api(test : true);
 *   var url = api.url("rest/myApi/", "my-dummy-data.json");
 */
class Api {
  bool test;
  
  Api({bool test: false}) {
    this.test = test;
  }
  
  String url(String url, [devUrl]) {
   if (isDartium()) {
     if (test && devUrl != null) {
       return devUrl;
     }
     // dartium dev mode 
     return "http://localhost:12000/sipxconfig/${url}";
   }  
   return "/sipxconfig/${url}";
  }
  
  bool isDartium() {
    return (window.location.port == '3030');    
  }
  
  String baseUrl() {
    if (isDartium()) {
      var baseUrl = new RegExp(r"(.*/context)");
      return baseUrl.stringMatch(window.location.href);
    }
    return "/sipxconfig";
  }
}

/**
 * S T R I N G  R E S O U R C E S
 */
/**
 * Gets a resource string from a coresponding tapestry component on the
 * page.
 *   getString("bird");
 * Looks at
 *   <span jwcid="@common/Message" key="bird">Bird is the word.</span>
 *   
 * Optional second arg is a list of parameters to insert into string and
 * replace {N} tokens where N is integer.
 */
String getString(String rcId, [List<String> args]) {
  if (strings == null) {
    _loadStrings();
  }
  // support non-standard but convenient format where if key has
  // spaces it will split on space and treat everything after first
  // space as a parameter
  if (rcId.contains(" ", 0)) {
    var idAndArgs = rcId.split(" ");
    return getString(idAndArgs[0], idAndArgs.sublist(1));
  }
  String rc = strings[rcId];
  if (rc == null) {
    return rcId;
  }
  if (args != null) {
    for (var i = 0; i < args.length; i++) {
      rc = rc.replaceAll("{$i}", args[i]);        
    }    
  }
  return rc;
}

Map<String,String> strings;
_loadStrings() {
  strings = new HashMap<String,String>();
  for (SpanElement e in querySelector("#rc").children) {
    strings[e.attributes["key"]] = e.text;    
  }
}

/**
 * Manage a set of links in the left-hand side of the page that flips between
 * divs emulating that user is navigating between different pages.  Widget assumes
 * a naming convention.
 * 
 * Convention: For each tab id "foo" passed in constructor, there exists 
 * a.) a clickable html element with id "foo-tab-link" (typically an anchor w/href)
 * b.) an html element with id "foo-tab" (typically an "li") that holds the link 
 * c.) an html element with id "foo" (typically an "div") element that holds the tab content
 * 
 * Features:
 * 1.) Can optionally call callback when a tab is changed, otherwise default behavaior
 * is to simply change CSS to display or not display divs. This is useful if page needs
 * to trigger page refresh or put scripts on the table to sleep on unload
 * 
 * 2.) Can optionally save which tab is active in client web browser storage for an 
 * http session.
 * 
 * NOTE: This does not render any html, only coordinates the on/off state of tabs.
 * 
 * Example html code:
 *  <ul id="mytabs">
 *    <li id=foo-tab><a href=# id=foo-tab-link>Foo</a></li>
 *    <li id=bar-tab><a href=# id=bar-tab-link>Bar</a></li>
 *  <ul>
 *  <div id=foo>
 *  </div>
 *  <div id=bar style="display:none">
 *  </div>
 *  
 * Example dart code
 *  main() {
 *    new Tabs(querySelector("#mytabs"), ["foo", "bar"]);
 *  }
 */
class Tabs {
  Element root;
  List<String> ids;
  TabCallback tabChangeListener;
  String persistentStateId;
  
  Tabs(Element this.root, Iterable<String> ids, [this.tabChangeListener]) {
    this.ids = ids.toList(growable: false);
    for (String id in this.ids) {
      var linkId = "#${id}-tab-link";
      AnchorElement link = root.querySelector(linkId);
      if (link == null) {
        print("ERROR: Cannot find tab link ${linkId}");
        continue;
      }
      link.onClick.listen((_) {
        changeTab(id);
      });      
    }    
  }
  
  changeTab(String selectedId) {    
    if (tabChangeListener != null) {
      tabChangeListener(selectedId);
    }
    for (String id in this.ids) {
      var tabId = "${id}-tab";
      Element tab = root.querySelector("#${tabId}");
      if (tab == null) {
        print("ERROR: Cannot find tab with id ${tabId}");
      } else {
        if (id == selectedId) {
          tab.classes.add("active");        
        } else {
          tab.classes.remove("active");                
        }
      }
      
      Element tabContent = querySelector("#${id}");
      if (tabContent == null) {
        print("ERROR: Cannot find tab ${id}");
      } else {
        if (id == selectedId) {
          tabContent.style.display = "";
        } else {
          tabContent.style.display = "none";
        }
      }
    }
    persistActiveTabId(selectedId);
  }
  
  persistActiveTabId(String tabId) {
    if (persistentStateId != null) {
      window.sessionStorage[persistentStateId] = tabId;      
    }
  }
  
  setPersistentStateId(String persistenceId) {
    persistentStateId = persistenceId;
    var store = window.sessionStorage;
    if (window.sessionStorage.containsKey(persistentStateId)) {      
      changeTab(store[persistentStateId]);      
    }
  }
}
typedef void TabCallback(String id);


/**
 * Calls listener every N seconds. Useful for pages that have a refresh widget
 * that refreshed the page.
 *  
 * Features:
 * 1.) Will update UserMessage object if there are any errors
 * 2.) Save refresh rate in client-side http session
 * 3.) Restart timer of page is refreshed
 */
class Refresher {
  CheckboxInputElement on;
  int refreshRate = 30;
  RefreshCallback listener;
  Timer timer;
  
  Refresher(Element parent, ButtonElement button, [void listener()]) {
    this.listener = listener;
    on = new CheckboxInputElement();
    on.onChange.listen((e) {
      conditionalStart();
    });

    String id = parent.id;
    if (id != null) {
      on.checked =  window.sessionStorage["${id}.enabled"] != "false";
    } else {
      on.checked = true;      
    }
    
    var refreshRateElem = new TextInputElement();
    refreshRateElem.size = 2;
    refreshRateElem.value = refreshRate.toString();
    refreshRateElem.onChange.listen((e) {
      refreshRate = int.parse(refreshRateElem.value);
      conditionalStart();
    });
    parent.children.add(on);
    parent.appendText(getString("refreshEvery") + " ");
    parent.children.add(refreshRateElem);
    parent.appendText(" " + getString("seconds"));
    if (button != null) {
      button.onClick.listen((e) => refresh());
    }
  }
  
  void refresh() {
    // when button is pressed or refresh call programatically
    // we want to restart the timer
    try {
      stop();
      if (listener != null) {
        listener();
      }
    } finally {
      conditionalStart();
    }    
  }
  
  void stop() {
    if (timer != null) {
      timer.cancel();
    }      
  }
  
  void conditionalStart() {
    stop();
    if (on.checked) {
      timer = new Timer.periodic(new Duration(seconds: refreshRate), (e) {
        if (listener != null) {                                                                            
          listener();
        }
      });
    }
  }
}
typedef void RefreshCallback();



/**
 * Makes GET request to REST API and checks for errors, on no
 * error, call given listener.
 */
class DataLoader {
  UserMessage msg;
  DataLoaderCallback listener;
  bool confirmErrors;
  
  DataLoader(UserMessage msg, void listener(String), [bool this.confirmErrors = false]) {
    this.msg = msg;
    this.listener = listener;
  }
  
  void load(url) {
    print("loading data");
    Future<String> request = HttpRequest.getString(url);
    request.then(this.listener, onError: (e) {
      checkResponse(msg, e.currentTarget, confirmErrors);
    });      
  }

  static bool checkResponse(UserMessage msg, HttpRequest request, [bool confirmErrors = false]) {
    if (request.status == 200) {
      return true;
    }
    String userError;
    try {
      userError = JSON.decode(request.responseText)['error'];
    } catch(notJson) {      
    }
    if (userError == null) {
      if (request.status == 0) {
        userError = "Disconnected from server";
      } else {
        userError = "${request.status} Error";
      }
    }
    if (confirmErrors) {
      msg.errorConfirm(userError);
    } else {
      msg.error(userError);          
    }
    return false;
  }
}

typedef void DataLoaderCallback(String data);

/**
 * Simple paragraph element that shows success or error messages.
 * Features:
 * 
 * 1.) Widget can be used as many times as you want on a page w/o conflict typically
 * each element has a unique id, but that's up to you
 * 
 * 2.) Can ask for confirmation for error messages, useful for pages that refresh
 * automatically and error would be flushed.
 * 
 * Example:
 * 
 * main() {
 *    var msg = new UserMessage(querySelector("#my-message"));
 *    msg.success("Have a great day!");
 * }
 */
class UserMessage {
  Element msg;
  Element close;
  bool confirmError = false;
  UserMessage(Element parent) {    
    msg = new SpanElement();
    parent.children.add(msg);
    close = new ButtonElement();
    close.classes.add("close-popup");
    parent.children.add(close);
    close.style.display = "none";
    close.text = "ok";
    close.onClick.listen(clearError);
  }
  
  void success(String msg) {
    if (confirmError == false) {
      message(msg, 'user-success');
    }
  }
  
  void error(String msg) {
    message(msg, 'user-error');  
  }
  
  /**
   * If you display an error message on a page that gets refreshed automatically
   * your error message will disappear if subsequent success message is called.
   * Calling this error message will keep error message on page until user clicks
   * on close button or leaves page.
   */
  void errorConfirm(String msg) {
    message(msg, 'user-error');
    close.style.display = "";
    confirmError = true;
  }
  
  void clearError([e]) {
    confirmError = false;
    close.style.display = "none";
    success('');
  }

  void warning(String msg) {
    message(msg, 'user-warning');  
  }

  void internalError(int status) {
    error("${status} Error");
  }
  
  void message(String text, String css) {
    msg.text = text;
    msg.classes.clear();
    msg.classes.add(css);    
  }  
}
