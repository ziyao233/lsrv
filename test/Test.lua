--[[
--	lsrv
--	File:/tests/Test.lua
--	Date:2022.08.30
--	By MIT License.
--	Copyright (c) 2022 Ziyao.All rights reserved.
--]]

if LSRV_CONF
then
	return {
			workerNum = 2,
			port	  = 11451,
			workPath  = "/srv"
	       };
end
