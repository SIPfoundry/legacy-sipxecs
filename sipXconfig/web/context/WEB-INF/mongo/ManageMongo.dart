import 'dart:html';
import 'dart:json';
import 'dart:async';
import 'package:sipxconfig/sipxconfig.dart';

bool inProgressFlag = false;
ManageGlobal global = new ManageGlobal();
ManageLocal local = new ManageLocal();
List<ManageBase> all = [global, local];
ManageBase manage = global;
Tabs tabs;
var api = new Api(test : false);

main() {
  tabs = new Tabs(query("#leftNav"));
  tabs.add('global', getString('tab.global'), query("#global"), global);
  tabs.add('local', getString('tab.local'), query("#local"), local);
  tabs.onClick((id, token) {
    manage = token;
    for (var base in all) {
      if (manage == base) {
        base.load();
      } else {        
        base.unload();
      }
    }    
    for (var help in query(".help").children) {
      help.style.display = (manage.help == help.id ? "" : "none");
    }
    reload();
  });
  reload();
}

reload() {
  tabs.render();  
  manage.load();  
}

abstract class ManageBase {
  UserMessage msg;
  String help;
  void load();
  void unload();
  void onServerAction(String label, String server, String action);
}

class ManageGlobal extends ManageBase {
  Refresher refresh;
  DataLoader loader;
  UiBuilder builder;
  var help = "global.help";
  
  ManageGlobal() {
    help = "global.help";
    msg = new UserMessage(query("#globalMessage"));    
    loader = new DataLoader(msg, loadTable);
    builder = new UiBuilder(this);    
    refresh = new Refresher(query("#globalRefreshWidget"), query("#globalRefreshButton"), () {
      var url = api.url("rest/mongoGlobal/", "global-test.json");
      loader.load(url);      
    });
  }
 
  void load() {
    builder.inProgress(true);    
    refresh.refresh();
  }
  
  void unload() {
    refresh.stop();
  }
  
  void loadTable(data) {    
    var meta = parse(data);    
    builder.addMongoNodeSelect(meta['dbCandidates'], query('#globalAddDb'), 'NEW_DB', getString('addDatabase'));
    builder.addMongoNodeSelect(meta['arbiterCandidates'], query('#globalAddArbiter'), 'NEW_ARBITER', getString('addArbiter'));
    TableSectionElement tbody = query("#globalTable");
    tbody.children.clear();
    builder.lastError(meta['lastConfigError']);    
    var rows = new List<TableRowElement>();
    for (var type in ['databases', 'arbiters']) {
      if (meta[type] == null) {
        continue;
      }
      meta[type].forEach((server, node) {
        var row = builder.row();        
        builder.nameColumn(row.addCell(), node, server, type);                
        builder.statusColumn(row.addCell(), node, server, type);
        builder.actionColumn(row.addCell(), node, server, type);
        rows.add(row);
      });
    }    
    rows.sort((TableRowElement a, TableRowElement b) {
      return compare(a.cells[0].text, b.cells[0].text);
    });    
    tbody.children.addAll(rows);
    builder.inProgress(meta['inProgress']);
  }
  
  void onServerAction(String label, String server, String action) {    
    var httpRequest = new HttpRequest();
    httpRequest.open('POST', api.url("rest/mongoGlobal/"));
    httpRequest.onLoadEnd.listen((e) {
      if (DataLoader.checkResponse(msg, httpRequest)) {
        load();
      }
    });
    var post = stringify({'server' : server, 'action' : action });
    httpRequest.send(post);
  }  
}

class ManageLocal extends ManageBase {
  Refresher refresh;
  DataLoader loader;
  UiBuilder builder;
  
  ManageLocal() {
    help = "local.help";
    msg = new UserMessage(query("#localMessage"));    
    loader = new DataLoader(msg, loadTable);
    builder = new UiBuilder(this);
    refresh = new Refresher(query("#localRefreshWidget"), query("#localRefreshButton"), () {
      var url = api.url("rest/mongoRegional/", "local-test.json");
      loader.load(url);      
    });
  }
      
  void loadTable(data) {
    var meta = parse(data);
    TableSectionElement tbody = query("#localTable");
    tbody.children.clear();
    builder.lastError(meta['lastConfigError']);  
    List candidates = meta['dbCandidates'];
    builder.addMongoNodeSelect(candidates, query('#localAddDb'), 'NEW_LOCAL', getString('addDatabase'));
    builder.addMongoNodeSelect(meta['arbiterCandidates'], query('#localAddArbiter'), 'NEW_LOCAL_ARBITER', getString('addArbiter'));
    
    List shards = meta['shards'];
    if ((shards == null || shards.length == 0) && (candidates == null || candidates.length == 0)) {
      msg.warning('''
Only servers with regions defined can host a local database. Assign 
regions to servers if you wish to have a local databbase
''');
      builder.inProgress(false);
      return;
    }      
      
    var rows = new List<TableRowElement>();
    for (var shard in shards) {
      var count = 0;
      for (var type in ['databases', 'arbiters']) {
        if (shard[type] != null) {
          count += shard[type].length;
        }
      }
      for (var type in ['databases', 'arbiters']) {
        if (shard[type] == null) {
          continue;
        }
        shard[type].forEach((server, node) {
          var row = builder.row();
          builder.nameColumn(row.addCell(), node, server, type);
          builder.statusColumn(row.addCell(), node, server, type);
          String region = node['region'].toString();
          row.addCell().text = (region == null  ? '' : region);
          var actions = builder.actionColumn(row.addCell(), node, server, type);
          if (count == 1) {
            // Delete on last database is different server side command
            // but to user they do not need to know that so keep label
            // the same as removing a database
            var cmd = (type == 'databases' ? 'DELETE_LOCAL' : 'DELETE_LOCAL_ARBITER'); 
            actions.children.add(new OptionElement(getString('action.DELETE'), cmd, false, false));
          }
          rows.add(row);
        });
      }
    }
    rows.sort((TableRowElement a, TableRowElement b) {
      int order = compare(a.cells[2].text, b.cells[2].text);
      if (order != 0) {
        return order;
      }
      return compare(a.cells[0].text, b.cells[0].text);
    });
    tbody.children.addAll(rows);
    builder.inProgress(meta['inProgress']);
  }
  
  void onServerAction(String label, String server, String action) {    
    var httpRequest = new HttpRequest();
    httpRequest.open('POST', api.url("rest/mongoRegional/"));
    httpRequest.onLoadEnd.listen((e) {
      if (DataLoader.checkResponse(msg, httpRequest)) {
        load();
      }
    });
    var post = stringify({'server' : server, 'action' : action });
    httpRequest.send(post);
  }  

  void load() {
    builder.inProgress(true);    
    refresh.refresh();
  }

  void unload() {
    refresh.stop();
  }
}

/**
 * Build the HTML Components common to multiple tabs 
 */
class UiBuilder {
  ManageBase manage;
  
  UiBuilder(ManageBase manage) {
    this.manage = manage;
  }
  
  void nameColumn(Element cell, node, server, type) {
    var img = new ImageElement();
    var src = statusImage(node['status']);
    img.src = "${api.baseUrl()}/images/${src}";
    cell.append(img);
    String typeText = dbType(type);      
    cell.appendText(" ${node['host']} ${typeText}");  
  }

  void statusColumn(Element cell, node, server, type) {
    var statusUl = new UListElement();
    cell.append(statusUl);
    for (String status in node['status']) {
      listItem(statusUl, status);
    }
    if (node['priority'] != null) {
      listItem(statusUl, getString('label.priority') + node['priority'].toString());
    }
    String voting = node['voting'] == false ? "disabled" : "enabled";
    listItem(statusUl, getString('label.voting') + voting);
  }

  SelectElement actionColumn(Element cell, node, server, type) {
    UListElement ul = new UListElement();
    cell.children.add(ul);        
    if (node['required'].length > 0) {
      ButtonElement required = new ButtonElement();
      for (var a in node['required']) {
        var label = getString('action.${a}');
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

    actions.children.add(new OptionElement(getString('options'), '', false, false));
    if (!(node['status'] as List<String>).contains('PRIMARY')) {
      actions.children.add(new OptionElement(getString('action.DELETE'), 'DELETE', false, false));
    } else {
      actions.children.add(new OptionElement(getString('action.STEP_DOWN'), 'STEP_DOWN 60', false, false));        
    }
    if (type != 'arbiters') {
      var priority = 1;
      if (node['priority'] != null) {
        priority = node['priority'];
      }
      actions.children.add(new OptionElement(getString('action.ADD_VOTE'), 'ADD_VOTE', false, false));
      actions.children.add(new OptionElement(getString('action.REMOVE_VOTE'), 'REMOVE_VOTE', false, false));
      actions.children.add(new OptionElement(getString('action.INCREASE_PRIORITY'), 'CHANGE_PRIORITY ${priority + 1}', false, false));
      actions.children.add(new OptionElement(getString('action.DECREASE_PRIORITY'), 'CHANGE_PRIORITY ${priority - 1}', false, false));
    }
    for (var a in node['available']) {
      var option = new OptionElement(getString('action.${a}'), a, false, false);
      actions.append(option);
    }
    
    return actions;
  }

  addMongoNodeSelect(List<String> candidates, Element parent, String action, String noneSelectedLabel) {   
    parent.children.clear();
    if (candidates == null || candidates.length <= 0) {
      return;
    }
    var addNode = new SelectElement();
    addNode.classes = ['action'];
    addNode.onChange.listen((e) {
      onServerAction(noneSelectedLabel, addNode.value, action);
    });
    addNode.append(new OptionElement(noneSelectedLabel, "", false, true));
    for (var candidate in candidates) {
      addNode.append(new OptionElement(candidate, candidate, false, false));
    }
    parent.append(addNode);
  }

  String optionLabel(SelectElement e, String value) {
    for (OptionElement o in e.options) {
      if (o.value == value) {
        return o.label;
      }
    }
    return value;
  }

  void listItem(Element ulist, String label) {
    var li = new LIElement();
    li.appendText(label);
    ulist.children.add(li);
  }

  var count = 0;

  String dbType(String type) {
    switch (type) {
      case 'databases':
        return getString('type.db');
      case 'arbiters':
        return getString('type.arbiter');
    }
  }

  String statusImage(List<String> status) {
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

  TableCellElement cell(String text) {
    var c = new TableCellElement();
    c.appendText(text);
    return c;
  }

  TableRowElement row() {
    var r = new TableRowElement();
    r.classes.add((++count % 2) == 0 ? 'even' : 'odd');
    return r;
  }

  ButtonElement addButton(UListElement list, String text) {
    var li = new LIElement();
    ButtonElement b = new ButtonElement();
    b.appendText(text);
    li.append(b);
    list.children.add(li);  
    return b;
  }

  inProgress(bool inProg) {
    query('#inprogress').hidden = ! inProg;
    for (var e in queryAll('.action')) {    
      e.disabled = inProg;
    }
  }
  
  lastError(String err) {
    if (err != null) {
      manage.msg.error(err);
    } else {  
      manage.msg.success("");
    }    
  }
  
  onServerAction(String label, String server, String action) {
    var msg = getString('confirmOperation', [server, label]);
    if (!window.confirm(msg)) {
      return;
    }
    inProgress(true);
    manage.onServerAction(label, server, action);    
  }
}

int compare(a, b) {
  if (a == null) {
    if (b == null) {
      return 0;
    }
    return -1;
  }
  return a.toString().compareTo(b.toString());
}
