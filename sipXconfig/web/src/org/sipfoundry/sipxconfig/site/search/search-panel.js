var searchPanel = {
    // property: placeholder
    // label that is displayed inside of the search box when it's not active
    placeholder:null,

    /**
     * Function: register
     * 
     * Installs onclick and onblur handlers.
     * Fixes Safari problem with styled controls.
     * Initialized display value to placeholder.
     */
    register:function(searchBox, placeholder) {
        this.placeholder = placeholder
        searchBox.value = this.placeholder;

        if (navigator.userAgent.indexOf("Safari") < 0) {
            dojo.byId('searchContainer').className = "styled";
        }

        dojo.event.connect(searchBox, "onclick", this, "onClick");
        dojo.event.connect(searchBox, "onblur", this, "onBlur");
    },

    onClick:function(evt) {
        var sb = evt.target;
        if (sb.value == this.placeholder) {
            sb.value = "";
        }
        sb.className = "active";
    },

    onBlur:function(evt) {
        var sb = evt.target;
        if (sb.value == "") {
            sb.value = this.placeholder;
            sb.className = "inactive";
        }
    }
}
