import 'dart:html';
import 'dart:json';
import 'package:sipxconfig/sipxconfig.dart';

ManageRegions regions = new ManageRegions();
var api = new Api(test : false);

main() {
  regions.reload();
}

/**
 * Add and edit regions 
 */
class RegionEditor {
  UserMessage msg;
  ManageRegions parent;
  Map<String, Object> region;
  
  RegionEditor(ManageRegions parent) {
    this.parent = parent;
    query("#add-region-link").onClick.listen(add);
    query("#region-save").onClick.listen(save);
    query("#region-cancel").onClick.listen(close);
    query("#region-ip-more").onClick.listen((_) {
      addIpaddr();
    });
    msg = new UserMessage(query("#editMessage"));
  }
  
  toggle([e]) {
    if (popup().style.display == "") {
      close();
    } else {
      open();
    }
  }
  
  edit(Map<String, Object> region) {
    this.region = region;
    reset();
    name().value = region['name'];
    List<String> addresses = region['addresses'];
    if (addresses != null && addresses.length > 0) {
      ipaddr0().value = addresses[0];
      for (int i = 1; i < addresses.length; i++) {
        addIpaddr(addresses[i]);
      }
    } else {
      ipaddr0().value = null;      
    }
    open();
  }
  
  add([e]) {
    this.region = null;
    name().value = null;
    ipaddr0().value = null; 
    reset();
    open();
  }
  
  save([e]) {
    HttpRequest req = new HttpRequest();
    var meta = {"name": name().value, "addresses" : addresses() };
    var method;
    if (region != null) {
      meta['id'] = region['id'];
      method = 'PUT';
    } else {
      method = 'POST';      
    }
    req.open(method, api.url("rest/region/"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send(stringify(meta));
    req.onLoad.listen((e) {
      if (DataLoader.checkResponse(msg, req)) {
        parent.reload();
        close();
      }
    }).onError((e) {
      msg.error(e.toString());      
    });      
  }
  
  List<String> addresses() {
    var addresses = [ ipaddr0().value ];
    for (InputElement ipaddr in ipaddrs()) {
      String value = ipaddr.value;
      if (value != null && value.trim().length > 0) {
        addresses.add(value);
      }      
    }

    return addresses;
  }
  
  reset() {
    query("#region-additional").children.clear();        
  }
  
  Element popup() {
    return query("#add-region-popup");
  }
  
  open([e]) {
    popup().style.display = "";    
  }
  
  close([e]) {
    popup().style.display = "none";    
  }
  
  InputElement name() {
    return query("#region-name") as InputElement;
  }
  
  InputElement ipaddr0() {
    return query("#region-ip-0") as InputElement;    
  }
  
  List<InputElement> ipaddrs() {
    return (query("#region-additional").queryAll("input") as List<InputElement>);    
  }
  
  addIpaddr([String address]) {
    Element rows = query("#region-additional");
    var addRow = new TableRowElement();
    addRow.addCell();
    var cell = addRow.addCell();
    var addName = new InputElement();
    addName.value = address;
    addName.autofocus = true;
    var x = new ButtonElement();
    x.text = 'X';
    x.onClick.listen((_) {
      rows.children.remove(addRow);
    });
    cell.children.add(addName);
    cell.children.add(x);
    rows.children.add(addRow);       
  }
}

/**
 * Manage lists of regions and delete existing regions
 */
class ManageRegions {
  UserMessage msg;
  DataLoader loader;
  RegionEditor editor;
  
  ManageRegions() {
    msg = new UserMessage(query("#userMessage"));    
    loader = new DataLoader(msg, loadTable);
    editor = new RegionEditor(this);
  }
    
  void removeRegion(int region, String name) {
    if (window.confirm("Are you sure you want to remove ${name}?")) {
      HttpRequest req = new HttpRequest();
      req.open('DELETE', api.url("rest/region/${region}/"));
      req.setRequestHeader("Content-Type", "application/json");      
      req.send();
      req.onLoad.listen(reload, onError: onError);
    }
  }
  
  void onError(e) {
    msg.error(e.toString());    
  }
  
  void reload([HttpRequestProgressEvent event]) {
    if (event != null) {
      HttpRequest req = event.target; 
      if (req.status != 200) {
        var err = parse(req.responseText);
        msg.error(err['error']);            
      }
    }
    loader.load(api.url("rest/region/", 'regions-test.json'));
  }
     
  void loadTable(data) {    
    List regions = parse(data);  
    TableSectionElement tbody = query("#regionTable");
    tbody.children.clear();
    for (var region in regions) {
      TableRowElement row = tbody.addRow();
      var nameCell = row.addCell();
      AnchorElement link = new AnchorElement(href: '#');
      link.text = region['name'];
      link.onClick.listen((_) {
        editor.edit(region);
      });
      nameCell.children.add(link);
      row.addCell().text = region['servers'].join(', ');
      ButtonElement remove = new ButtonElement();
      remove.text = "Remove";
      remove.onClick.listen((_) {
        removeRegion(region['id'], region['name']);        
      });
      row.children.add(remove);
      tbody.children.add(row);
    };
  }
}

