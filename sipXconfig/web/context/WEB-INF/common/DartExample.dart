import 'dart:html';
import 'dart:json';
import 'dart:async';

String baseurl = "";
String jsonurl = "";

main() {
  if (query("#devmode") != null) {
    baseurl = "http://localhost:12000";
  }
  jsonurl = "${baseurl}/sipxconfig/rest/birds";

  // Test Data
  // var workdir = "home/dhubler/work/sipxecs/sipXconfig";
  // jsonurl = "http://127.0.0.1:3030/${workdir}/neoconf/test/org/sipfoundry/sipxconfig/common/dart-example.json";
  
  getData();
  
  query("#reset").onClick.listen((e) {reset();});
  query("#add").onClick.listen((e) {
    String day = query("#day").value;
    String bird = query("#bird").value;
    addSighting(day, bird);
  });
  
  DateTime now = new DateTime.now();
  query("#day").value = "${now.month}/${now.month}/${now.year}";
}

successMessage(String msg) {
  Element msg = query('#message');
  msg.text = "refresh rate changed";
  msg.classes = [ 'user-success' ];  
}

getData() {
  Future<String> request = HttpRequest.getString(jsonurl);
  request.then((data) {loadTable(data);});
}

loadTable(json) {
  var rows = query("#spottings-data");
  rows.children.clear();  
  var data = parse(json);
  data['spottings'].forEach((day, birds) {
    var row = _row();
    
    row.addCell().appendText(day);
    
    row.addCell().append(_list(birds));
    
    ButtonElement b = new ButtonElement();
    b.appendText("Remove");
    b.onClick.listen((e) {
      removeSighting(day);      
    });
    row.addCell().append(b);
    
    rows.children.add(row);
  });
}

ButtonElement addButton(String text) {
  var li = new LIElement();
  ButtonElement b = new ButtonElement();
  b.appendText("Delete");
  return b;
}

reset() {
  successMessage("Got here");
  var post = stringify({'action' : 'RESET'});
  onServerAction(post);  
}

addSighting(day, bird) {
  var post = stringify({'action' : 'ADD', 'day' : day, 'bird' : bird});
  onServerAction(post);
}

removeSighting(String day) {
  var post = stringify({'action' : 'DELETE', 'day' : day});
  onServerAction(post);      
}

onServerAction(String postData) {
  var httpRequest = new HttpRequest();
  httpRequest.open('POST', jsonurl);
  httpRequest.onLoadEnd.listen((e) => checkResponse(httpRequest));
  httpRequest.send(postData);
  successMessage("Success");
  getData();
}

checkResponse(Object request) {
  var msg = query("#message");
  if (request.status != 200) {
    msg.text = 'Uh oh, there was an error of ${request.status}';
  } else {
    msg.text = 'Data has been posted';
  }  
}

var count = 0;

UListElement _list(List<String> items) {
  UListElement list = new UListElement();
  for (var item in items) {
    LIElement li = new LIElement();
    li.text = item;
    list.children.add(li);
  }
  return list;
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
