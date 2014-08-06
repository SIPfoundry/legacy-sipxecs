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
import 'dart:html';
import 'dart:convert';
import 'package:sipxconfig/sipxconfig.dart';

var api = new Api(test : false);

main() {
  var tabIds = [ "settings", "views", "failover", "custom", "advisor", "advisor-regions" ];
  var tabs = new Tabs(querySelector("#leftNavAbsolute"), tabIds);
  tabs.setPersistentStateId("editDns");
  new Views();
  new Failover();
  new Custom();
}

class Failover {
  var msg = new UserMessage(querySelector("#failover-message"));
  DataLoader loader;
  
  Failover() {
    querySelector("#failover-delete").onClick.listen(deleteSelected);
    loader = new DataLoader(this.msg, loadTable);
    load();
  }
  
  load() {
    var url = api.url("rest/dnsPlan/", "failover-test.json");
    loader.load(url);    
  }
  
  deleteSelected(event) {
    if (!window.confirm(getString("msg.deletePlansConfirm"))) {
      return;
    }
    var ids = selectedIds("selected-plans");
    for (int id in ids) {
      HttpRequest req = new HttpRequest();
      req.open('DELETE', api.url("rest/dnsPlan/${id}"));
      req.send();
      req.onLoadEnd.listen((_) {
        if (DataLoader.checkResponse(msg, req)) {
          if (id == ids.last) {
            load();
          }
        }
      });
    }
  }  

  loadTable(data) {
    var tbody = querySelector("#failover-items");
    tbody.children.clear();
    var meta = JSON.decode(data);
    for (var plan in meta) {
      List views = plan['views'];
      String viewsHtml = (views == null ? '' : views.join(', '));
      var e = new Element.html('''
<table>
  <tbody>
    <tr>
      <td><input type="checkbox" name="selected-plans" value="${plan['id'].toString()}"/></td>
      <td><a href="EditDnsPlan.html?dnsPlanId=${plan['id'].toString()}">${plan['name']}</a></td>
      <td>${viewsHtml}</td>
    </tr>
  </tbody>
</table>
''');
        tbody.children.add(e.querySelector("tr"));   
    }
  }  
}

class Views {
  var msg = new UserMessage(querySelector("#views-message"));
  DataLoader loader;
  List<int> selectIdsPostLoad;
  
  Views() {
    querySelector("#views-delete").onClick.listen(deleteSelected);
    querySelector("#views-up").onClick.listen((_) => moveSelected(-1));
    querySelector("#views-down").onClick.listen((_) => moveSelected(1));
    loader = new DataLoader(this.msg, loadTable);
    load();    
  }  
  
  moveSelected(int step) {
    var ids = selectedIds("selected-views");
    if (ids.length == 0) {
      return;
    }
    HttpRequest req = new HttpRequest();
    req.open('PUT', api.url("rest/dnsViewMove"));
    var data = JSON.encode({"step" : step, "ids" : ids});
    req.send(data);
    req.onLoadEnd.listen((_) {
      load();
      // load is asychronous, so we cannot simply set selected here, 
      // instead we set class variable to denote they need to be
      // set when data comes in.
      selectIdsPostLoad = ids;
    });
  }
  
  deleteSelected(event) {
    if (!window.confirm(getString("msg.deleteViewsConfirm"))) {
      return;
    }
    var ids = selectedIds("selected-views");
    for (int id in ids) {
      HttpRequest req = new HttpRequest();
      req.open('DELETE', api.url("rest/dnsView/${id}"));
      req.send();
      req.onLoadEnd.listen((_) {
        if (id == ids.last) {
          load();
        }
      });
    }
  }  
  
  load() {
    var url = api.url("rest/dnsView/", "views-test.json");
    loader.load(url);        
  }

  loadTable(data) {
    var tbody = querySelector("#view-items");    
    tbody.children.clear();
    var def = new Element.html('''
    <table>
  <tbody>
        <tr>
      <td></td>
      <td><a href="EditDefaultView.html">default</a></td>
      <td></td>
      <td></td>
    </tr>
      </tbody>
</table>
    ''');
    tbody.children.add(def.querySelector("tr"));
    var meta = JSON.decode(data);
    for (var view in meta) {
      var e = new Element.html('''
<table>
  <tbody>
    <tr>
      <td><input type="checkbox" name="selected-views" value="${view['id'].toString()}"/></td>
      <td><a href="EditDnsView.html?dnsViewId=${view['id'].toString()}">${view['name']}</a></td>
      <td>${view['plan'] != null ? view['plan'] : ''}</td>
      <td>${view['region']}</td>
    </tr>
  </tbody>
</table>
''');
        tbody.children.add(e.querySelector("tr"));   
    }
    if (selectIdsPostLoad != null) {
      setSelectedIds("selected-views", selectIdsPostLoad);
      selectIdsPostLoad = null;
    }
  }  
}

setSelectedIds(String checkboxName, List<int> ids) {
  List<CheckboxInputElement> all = querySelectorAll("input[name=${checkboxName}]");  
  for (var e in all) {
    int value = int.parse(e.defaultValue);
    if (ids.contains(value)) {
      e.checked = true;
    }
  }  
}

class Custom {
  var msg = new UserMessage(querySelector("#custom-message"));
  DataLoader loader;

  Custom() {
    querySelector("#custom-delete").onClick.listen(deleteSelected);
    loader = new DataLoader(this.msg, loadTable);
    load();        
  }

  load() {
    var url = api.url("rest/dnsCustom/", "custom-test.json");
    loader.load(url);        
  }

  loadTable(data) {
    var tbody = querySelector("#custom-items");    
    tbody.children.clear();
    var meta = JSON.decode(data);
    for (var custom in meta) {
      List views = custom['views'];
      String viewsHtml = (views == null ? '' : views.join(', '));
      var e = new Element.html('''
<table>
  <tbody>
    <tr>
      <td><input type="checkbox" name="selected-custom" value="${custom['id'].toString()}"/></td>
      <td><a href="EditDnsCustom.html?dnsCustomId=${custom['id'].toString()}">${custom['name']}</a></td>
      <td>${viewsHtml}</td>
    </tr>
  </tbody>
</table>
''');
        tbody.children.add(e.querySelector("tr"));   
    }
  }

  deleteSelected(event) {
    if (!window.confirm(getString("msg.deleteCustomConfirm"))) {
      return;
    }
    var ids = selectedIds("selected-custom");
    for (int id in ids) {
      HttpRequest req = new HttpRequest();
      req.open('DELETE', api.url("rest/dnsCustom/${id}"));
      req.send();
      req.onLoadEnd.listen((_) {
        if (id == ids.last) {
          load();
        }
      });
    }
  }
}

/**
 * Utils
 */
List<int> selectedIds(String checkboxName) {
  var on = new List<int>();
  List<CheckboxInputElement> all = querySelectorAll("input[name=${checkboxName}]");
  for (var e in all) {
    if (e.checked) {
      on.add(int.parse(e.value));
    }
  }
  return on;
}
