%% Initial Version Copyright (C) 2010 eZuce, Inc., All Rights Reserved.
%% Licensed to the User under the LGPL license.
%% 
%% order is import for some of these as afar as building dependencies first
%% consult spec files for authority on dependencies before changing the order

-module(sipxplugin_supervisor).
-author("eZuce").

-behavior(supervisor).
-export([start_link/0, init/1]).

start_link() ->
	supervisor:start_link({local, ?MODULE}, ?MODULE, []).

init([]) ->
	{ok, {{one_for_one, 10, 1},
		 [
		  {sipxplugin_poller,
			{sipxplugin_poller, start, []},
			 permanent,
			 100,
			 worker,
			[sipxplugin_poller]}
		 ]
	}}.
