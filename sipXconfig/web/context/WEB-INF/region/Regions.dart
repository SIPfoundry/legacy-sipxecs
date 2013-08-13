import 'dart:html';
import 'dart:json';
import 'package:sipxconfig/sipxconfig.dart';

ManageRegions regions = new ManageRegions();
var api = new Api(test : true);

main() {
  regions.reload();
}

class ManageRegions {
  UserMessage msg;
  TableSectionElement table;
  DataLoader loader;
  
  ManageRegions() {
    msg = new UserMessage(query("#userMessage"));    
    table = dataTable(query("#regionTable"), [ getString('name'), getString('servers'), '']);
    loader = new DataLoader(msg, loadTable);
  }
    
  void removeRegion(int region, String name) {
    if (window.confirm("Are you sure you want to remove ${name}?")) {
      HttpRequest req = new HttpRequest();
      req.open('DELETE', api.url("rest/region/${region}"));
      req.setRequestHeader("Content-Type", "application/json"); 
      req.send();
      req.onLoad.listen(reload).onError((e) {
        window.alert(e.toString());      
      });
    }
  }
  
  void reload([event]) {
    loader.load(api.url("rest/region/", 'regions-test.json'));
  }
  
  void addRegion(Event event) {
    var name = (event.target as InputElement).value;
    HttpRequest req = new HttpRequest();
    req.open('POST', api.url("rest/region/"));
    req.setRequestHeader("Content-Type", "application/json"); 
    req.send('{"name":"${name}"}');
    req.onLoad.listen(reload).onError((e) {
      window.alert(e.toString());      
    });
  }
   
  void loadTable(data) {    
    List regions = parse(data);    
    table.children.clear();
    for (var region in regions) {
      var row = new Element.html('''
<tr>
 <td>${region['name']}</td>
 <td>${region['servers'].join(', ')}</td>
</tr>''');
      ButtonElement remove = new ButtonElement();
      remove.text = "Remove";
      remove.onClick.listen((_) {
        removeRegion(region['id'], region['name']);        
      });
      row.children.add(remove);
      table.children.add(row);
    };
    var addRow = new TableRowElement();
    var cell = addRow.addCell();
    var addName = new InputElement();
    addName.autofocus = true;
    addName.onChange.listen(addRegion);
    cell.children.add(addName);
    table.children.add(addRow);   
  }
}

