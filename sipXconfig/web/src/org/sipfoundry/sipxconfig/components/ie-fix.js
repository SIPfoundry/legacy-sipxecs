function hoverToOverForIE() {
    if (typeof document.body.style.maxHeight != "undefined") {
        // see: http://ajaxian.com/archives/detecting-ie7-in-javascript
        // modern browser: no need to simulate li:hover
        return;
    }

    function addOver() {
        this.className += " over";
    }

    function removeOver() {
        this.className = this.className.replace(" over", "");
    }

    if (document.getElementById) {
        var navRoot = document.getElementById("nav");
        for ( var i = 0; i < navRoot.childNodes.length; i++) {
            var node = navRoot.childNodes[i];
            if (node.nodeName == "LI") {
                node.onmouseover = addOver;
                node.onmouseout = removeOver;
            }
        }
    }
}
