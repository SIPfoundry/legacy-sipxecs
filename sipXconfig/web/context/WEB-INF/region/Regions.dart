import 'dart:html';
import 'dart:json';
import 'package:sipxconfig/sipxconfig.dart';

// developer aid. Path will have last developers work directory in it. darteditor 
// has no way to avoid this AFAICT 
String devBasePath = "http://127.0.0.1:3030/home/dhubler/work/sipxecs";
bool useStaticJson = true;

String baseurl;
ManageRegions regions = new ManageRegions();

main() {
  baseurl = devmode() ? "http://localhost:12000" : "";
  regions.reload();
}

class ManageRegions {
  UserMessage msg;
  TableSectionElement table;
  DataLoader loader;
  String jsonurl;
  
  ManageRegions() {
    jsonurl = "${baseurl}/sipxconfig/rest/region";
    if (devmode() && useStaticJson) {
      jsonurl = "${devBasePath}/sipXconfig/web/context/WEB-INF/region/regions-test.json";
    }
    msg = new UserMessage(query("#userMessage"));    
    table = dataTable(query("#regionTable"), [ getString('name'), '']);
    loader = new DataLoader(msg, loadTable);
  }
    
  void removeRegion(int region, String name) {
    if (window.confirm("Are you sure you want to remove ${name}?")) {
      //removeRegions();
    }
  }
  
  void reload() {
    loader.load(jsonurl);
  }
   
  void loadTable(data) {    
    List regions = parse(data);    
    table.children.clear();
    for (var region in regions) {
      var row = new Element.html('''
<tr>
 <td>${region['name']}</td>
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
    addName.onChange.listen((e) {
      window.alert((e.target as InputElement).value);
    });
    cell.children.add(addName);
    table.children.add(addRow);
    
  }
}

