%% Initial Version Copyright (C) 2011 eZuce, Inc., All Rights Reserved.
%% Licensed to the User under the LGPL license.
%%
%% This library is distributed in the hope that it will be useful, but WITHOUT
%% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
%% FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
%% details.

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
