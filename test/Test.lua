--[[
--	lsrv
--	File:/tests/Test.lua
--	Date:2022.10.28
--	By MIT License.
--	Copyright (c) 2022 Ziyao.All rights reserved.
--]]

if LSRV_CONF
then
	return {
			workerNum	= 1,
			listenPort	= 11451,
			workPath	= "/srv",
			listenIp	= "::1",
			maxConnection	= 8,
			backlog		= 8,
			mainFile	= "Test.lua",
			socketType	= 2,
	       };
end

return function()
	lsrv.write("Atom is cute!");
	return;
end
