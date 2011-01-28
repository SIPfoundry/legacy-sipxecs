%% Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
%% Licensed to the User under the LGPL license.
%% 
%% order is import for some of these as afar as building dependencies first
%% consult spec files for authority on dependencies before changing the order

-module(sipxplugin_app).
-author("eZuce").

-behavior(application).
-export([start/2, stop/1]).

start(_Type, _Args) ->
	sipxplugin_supervisor:start_link().


stop(_State) ->
	ok.
