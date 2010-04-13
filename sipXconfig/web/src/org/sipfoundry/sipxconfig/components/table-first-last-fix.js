/**
 * Modifies the CSS class of rows in the table by add 'first' and 'last'
 * classes. This is designed to work around the poor support for :first-child
 * and :last-child pseudo classes.
 */
function addFirstLast(table) {
    /**
     * Appends CSS class name (only if not used already)
     *
     * @param node that should be affected
     * @param cssClass css class name
     */
    function appendCssClass(el, cssClass) {
        if (el.className.indexOf(cssClass) < 0) {
            el.className += " " + cssClass;
        }
    }

    var rows = table.tBodies[0].rows;
    if (rows.length > 0) {
        appendCssClass(rows[0], 'first');
        appendCssClass(rows[rows.length - 1], 'last');
    }
}
