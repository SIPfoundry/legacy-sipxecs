import 'dart:html';
import 'dart:json';
import 'dart:async';
import 'dart:collection';

String baseurl = "";
String jsonurl = "";
Timer refreshTimer;
int refreshRate = 30;
Map<String,String> rcMap = new HashMap<String,String>();
bool inProgressFlag = false;

main() {
  loadResources();
  bool devmode = query("#devmode") != null; 
  if (devmode) {
    baseurl = "http://localhost:12000";
  }
  jsonurl = "${baseurl}/sipxconfig/rest/mongo";

  // Test Data
  if (devmode) {
    // developer aid. comment out before commiting. path will have last
    // developers work directory in it. darteditor has no way to avoid this 
    jsonurl = "http://127.0.0.1:3030/home/dhubler/work/sipxecs/sipXconfig/web/context/WEB-INF/mongo/test-data.json";
    //jsonurl = "https://mongo1.3zuce.com/sipxconfig/rest/mongo";
  }
  
  reloadData();
  query("#refreshRate").onChange.listen((e) {
    refreshRate = int.parse((query("#refreshRate") as TextInputElement).value);
    startTimer();
    successMessage("Refresh rate changed");    
  });
  query("#refresh").onClick.listen((e) {
    reloadData();
  });

  CheckboxInputElement e = query("#refreshEnabled");
  e.checked =  window.sessionStorage['refresh.mongo'] != "false";
  e.onChange.listen((e) {
    String sRefreshEnabled = e.target.checked ? "true" : "false";
    window.sessionStorage['refresh.mongo'] = sRefreshEnabled;
    startTimer();
  });
}

loadResources() {
  for (SpanElement e in query("#rc").children) {
    rcMap[e.attributes["key"]] = e.text;    
  }
}

successMessage(String msg) {
  pageMessage(msg, 'user-success');
}

errorMessage(String msg) {
  pageMessage(msg, 'user-error');
}

pageMessage(String text, String css) {
  Element msg = query('#message');
  msg.text = text;
  msg.classes = [ css ];    
}

getData() {
  print("loading data");
  Future<String> request = HttpRequest.getString(jsonurl);
  request.then((data) {loadTable(data);}, onError: (e) {checkResponse(e.currentTarget);});  
}

reloadData() {
  stopTimer();
  try {
    getData();
  } finally {
    startTimer();
  }
}

void stopTimer() {
  if (refreshTimer != null) {
    refreshTimer.cancel();
  }  
}

void startTimer() {
  stopTimer();
  CheckboxInputElement cb = query("#refreshEnabled");
  if (cb.checked) {
    refreshTimer = new Timer.periodic(new Duration(seconds: refreshRate), (t) {
        if (!inProgressFlag) {
          getData();
        }
    });
  }
}

addMongoNodeSelect(List<String> candidates, String domId, String action, String noneSelectedLabel) {
  Element e = query(domId);
  e.children.clear();
  if (candidates.length <= 0) {
    return;
  }
  var addNode = new SelectElement();
  addNode.classes = ['action'];
  addNode.onChange.listen((e) {
    onServerAction(noneSelectedLabel, e.target.value, action);
  });
  addNode.append(new OptionElement(noneSelectedLabel, "", false, true));
  for (var candidate in candidates) {
    addNode.append(new OptionElement(candidate, candidate, false, false));
  }
  e.append(addNode);
}

String optionLabel(SelectElement e, String value) {
  for (OptionElement o in e.options) {
    if (o.value == value) {
      return o.label;
    }
  }
  return value;
}

loadTable(data) {
  var nodes = query("#nodes");
  nodes.children.clear();
  var meta = parse(data);  
  addMongoNodeSelect(meta['arbiterCandidates'], '#newArbs', 'NEW_ARBITER', rc('addArbiter'));
  addMongoNodeSelect(meta['dbCandidates'], '#newDbs', 'NEW_DB', rc('addDatabase'));
    
  for (var type in ['databases', 'arbiters']) {
    meta[type].forEach((server, node) {
      var row = _row();
      var img = new ImageElement();
      var src = _statusImage(node['status']);
      img.src = "${baseurl}/sipxconfig/images/${src}";
      var nameCell = row.addCell();
      nameCell.append(img);
      String typeText = _type(type);      
      nameCell.appendText(" ${node['host']} ${typeText}");
      
      var statusUl = new UListElement();
      row.addCell().append(statusUl);
      for (String status in node['status']) {
        _listItem(statusUl, status);
      }
      var priority = 1;
      if (node['priority'] != null) {
        priority = node['priority'];
        _listItem(statusUl, rc('label.priority') + priority.toString());
      }
      
      String voting = node['voting'] == false ? "disabled" : "enabled";
      _listItem(statusUl, rc('label.voting') + voting);
      
      TableCellElement actionCell = row.addCell();
      UListElement ul = new UListElement();
      actionCell.children.add(ul);      
  
      if (node['required'].length > 0) {
        ButtonElement required = new ButtonElement();
        for (var a in node['required']) {
          var label = rc('action.${a}');
          ButtonElement b = addButton(ul, label);
          b.classes = ['action'];
          b.onClick.listen((e) {
            onServerAction(label, server, a);
          });
        }
      }
      
      SelectElement actions = new SelectElement();
      actions.classes = ['action'];
      ul.children.add(actions);
      actions.onChange.listen((e) {
        SelectElement select = (e.target as SelectElement);
        String label = optionLabel(select, select.value);
        onServerAction(label, server, select.value);
      });

      actions.children.add(new OptionElement(rc('options'), '', false, false));
      if (!(node['status'] as List<String>).contains('PRIMARY')) {
        actions.children.add(new OptionElement(rc('action.DELETE'), 'DELETE', false, false));
      } else {
        actions.children.add(new OptionElement(rc('action.STEP_DOWN'), 'STEP_DOWN 60', false, false));        
      }
      if (type != 'arbiters') {
        actions.children.add(new OptionElement(rc('action.ADD_VOTE'), 'ADD_VOTE', false, false));
        actions.children.add(new OptionElement(rc('action.REMOVE_VOTE'), 'REMOVE_VOTE', false, false));
        actions.children.add(new OptionElement(rc('action.INCREASE_PRIORITY'), 'CHANGE_PRIORITY ${priority + 1}', false, false));
        actions.children.add(new OptionElement(rc('action.DECREASE_PRIORITY'), 'CHANGE_PRIORITY ${priority - 1}', false, false));
      }
      for (var a in node['available']) {
        var option = new OptionElement(rc('action.${a}'), a, false, false);
        actions.append(option);
      }    
      nodes.children.add(row);
    });
  }
  inProgress(meta['inProgress']);
  
  var err = meta['lastConfigError'];
  if (err != null) {
    errorMessage(err);
  } else {  
    successMessage("");
  }
}

ButtonElement addButton(UListElement list, String text) {
  var li = new LIElement();
  ButtonElement b = new ButtonElement();
  b.appendText(text);
  li.append(b);
  list.children.add(li);  
  return b;
}

String rcArgs(String rcId, List<String> args) {
  String rc = rcMap[rcId];
  for (var i = 0; i < args.length; i++) {
    rc = rc.replaceAll("{$i}", args[i]);        
  }
  return rc;
}

String rc(String rcId) {
  if (rcId.contains(" ", 0)) {
    var idAndArgs = rcId.split(" ");
    return rcArgs(idAndArgs[0], idAndArgs.sublist(1));
  }
  String rc = rcMap[rcId];
  if (rc == null) {
    return rcId;
  }
  return rc;
}

inProgress(bool yesOrNo) {
  inProgressFlag = yesOrNo;
  query('#inprogress').hidden = ! yesOrNo;
  for (Element e in queryAll('.action')) {
    e.disabled = yesOrNo;
  }
}

onServerAction(String label, String server, String action) {
  var msg = rcArgs('confirmOperation', [server, label]);
  if (!window.confirm(msg)) {
    return;
  }
  inProgress(true);
  var httpRequest = new HttpRequest();
  var url = "${baseurl}/sipxconfig/rest/mongo";
  httpRequest.open('POST', url);
  httpRequest.onLoadEnd.listen((e) {
    checkResponse(httpRequest);
    reloadData();
  });
  var post = stringify({'server' : server, 'action' : action });
  httpRequest.send(post);
}  

checkResponse(HttpRequest request) {
  var msg = query("#message");
  if (request.status != 200) {
    String userError;
    try {
      userError = parse(request.responseText)['error'];
    } catch(notJson) {      
    }
    if (userError == null) {
      userError = rc('error.internal');
    }
    errorMessage(userError);
  } else {
    successMessage(rc('operation.success'));
  }  
}

void _listItem(Element ulist, String label) {
  var li = new LIElement();
  li.appendText(label);
  ulist.children.add(li);
}

var count = 0;

String _type(String type) {
  switch (type) {
  case 'databases':
    return rc('type.db');
  case 'arbiters':
    return rc('type.arbiter');
  }
}

String _statusImage(List<String> status) {
  for (var s in status) {
    switch (s) {
    case 'PRIMARY':
    case 'SECONDARY':
    case 'ARBITER':
      return 'running.png';
    case 'UNAVAILABLE':
      return 'cross.png';        
    }      
  }
  return 'unknown.png';
}

TableCellElement _cell(String text) {
  var c = new TableCellElement();
  c.appendText(text);
  return c;
}

TableRowElement _row() {
  var r = new TableRowElement();
  r.classes.add((++count % 2) == 0 ? 'even' : 'odd');
  return r;
}
