library sipxconfig;

import 'dart:html';
import 'dart:async';
import 'dart:collection';
import 'dart:json';

Map<String,String> strings;

loadStrings() {
  strings = new HashMap<String,String>();
  for (SpanElement e in query("#rc").children) {
    strings[e.attributes["key"]] = e.text;    
  }
}

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
    loadStrings();
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

typedef void TabCallback(String id, Object tabToken);
/**
 * Renders and controls a set of stacked vertical tabs, calls given listener
 * on tab selection for owner to excute function
 */
class Tabs {
  Element nav;
  Map<String, List<Object>> tabs = new LinkedHashMap();
  String activeTab;
  TabCallback listener;
  
  Tabs(Element parent) {
    nav = query("#leftNav");
    if (nav == null) {
      print("ERROR: No tabNavigation found on page");
    }
  }
  
  void add(String id, String label, Element div, Object token, [void callback(String id, Object tabToken)]) {
    if (activeTab == null) {
      activeTab = id;
    }
    tabs[id] = [label, div, token, callback];
  }
  
  void onClick(void callback(String id, Object tabToken)) {
    this.listener = callback;
  }
  
  void render() {
    if (nav == null) {
      return;
    }
    nav.children.clear();
    div(nav, 'roundedNavSectionBoxTopLeft');
    div(nav, 'roundedNavSectionBoxTopRight');
    var tabNav = div(nav, 'roundedNavSectionBoxInside');
    tabNav = span(tabNav, 'roundedSectionBoxInsideContent');
    var ul = new UListElement();
    ul.classes.add("htabs");
    tabNav.append(ul);
    tabs.forEach((tabId, tab) {
      var item = new LIElement();
      item.classes.add("tab");
      ul.append(item);
      item = div(item, 'tabLink');      
      if (activeTab == tabId) {
        item = div(item, 'shadow');
        item = div(item, 'active');
        if (tab[1] != null) {
          (tab[1] as Element).style.display = "";
        }
      } else {
        if (tab[1] != null) {
          (tab[1] as Element).style.display = "none";
        }
      }
      var link = new AnchorElement(href: "#");
      link.onClick.listen((e) {
        activeTab = tabId;
        var token = tab[2];
        if (tab[3] != null) {
          (tab[3] as TabCallback)(tabId, token);
        } else {
          listener(tabId, token);
        }
      });
      link.text = tab[0];
      item.append(link);
    });  
    div(nav, 'roundedNavSectionBoxBottomLeft');
    div(nav, 'roundedNavSectionBoxBottomRight');    
  }
}

// return tbody in case that's useful
TableSectionElement dataTable(parent, labels) {
  parent.children.clear();
  parent.classes.add('tableDiv');
  var e = span(parent, 'tableView');
  TableElement table = child(e, new TableElement(), 'component');
  var thead = table.append(table.createTHead());
  var row = thead.addRow();
  for (var label in labels) {
    row.append(new Element.html("<th>${label}</th>"));
  }  
  var tb = table.createTBody();
  table.append(tb);
  return tb;
}

Element child(parent, child, cls) {
  child.classes.add(cls);
  parent.append(child);
  return child;  
}

SpanElement span(parent, cls) {
  var e = new SpanElement();
  e.classes.add(cls);
  parent.append(e);
  return e;
}

DivElement div(parent, cls) {
  var e = new DivElement();
  e.classes.add(cls);
  parent.append(e);
  return e;
}

typedef void RefreshCallback();

/**
 * Calls listener every N seconds. 
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

typedef void DataLoaderCallback(String data);

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
    if (request.status != 200) {
      String userError;
      try {
        userError = parse(request.responseText)['error'];
      } catch(notJson) {      
      }
      if (userError == null) {
        msg.internalError(request.status);
      } else {
        if (confirmErrors) {
          msg.errorConfirm(userError);
        } else {
          msg.error(userError);          
        }
      }
      return false;
    }  
    return true;
  }
}

/**
 * Simple paragraph element that shows success or error messages.
 */
class UserMessage {
  Element msg;
  Element close;
  bool confirmError = false;
  UserMessage(Element parent) {    
    msg = new SpanElement();
    parent.children.add(msg);
    close = new SpanElement();
    close.classes.add("close-popup");
    parent.children.add(close);
    close.style.display = "none";
    close.text = "x";
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