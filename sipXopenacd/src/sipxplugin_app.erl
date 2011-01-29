%% Initial Version Copyright (C) 2011 eZuce, Inc., All Rights Reserved.
%% Licensed to the User under the LGPL license.
%%
%% This library is distributed in the hope that it will be useful, but WITHOUT
%% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
%% FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
%% details.

-module(sipxplugin_app).
-author("eZuce").

-behavior(application).
-export([start/2, stop/1]).

start(_Type, _Args) ->
	sipxplugin_supervisor:start_link().


stop(_State) ->
	ok.
