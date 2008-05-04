var sipxTableStyler = function(table, database) {
    table.className = 'component'
}

var sipxZebraStyler = function(item, database, tr) {
    if (tr.rowIndex % 2) {
        tr.className = 'even';
    } else {
        tr.className = 'odd';
    }
}
