local is51 = _VERSION == "Lua 5.1"
local isJit = jit ~= nil

-- Function copies
local setmetatable = setmetatable
local getmetatable = getmetatable
local pairs = pairs
local ipairs = ipairs
local type = type
local error = error
local assert = assert
local rawget = rawget
local rawset = rawset
local getfenv = getfenv
local setfenv = setfenv
local tostring = tostring
local newproxy = newproxy
local dofile = dofile
local loadfile = loadfile
local string = {
	len = string.len,
	find = string.find,
	sub = string.sub,
	format = string.format,
	rep = string.rep,
	match = string.match,
}
local table = {
	insert = table.insert,
}
local debug = {
	getinfo = debug.getinfo,
	getupvalue = debug.getupvalue,
	upvaluejoin = debug.upvaluejoin,
}

local m = {
	classRegistry = {}, -- Registry of all class definitions
	instanceRegistry = { -- Registry of all class instances
		classes = {},
		singletons = {},
	},

	classIndex = 0, -- Global counter for assigning class id numbers
	g = is51 and getfenv() or _G,

	classTypes = {
		TYPE_CLASS = 0,
		TYPE_SINGLETON = 1,
		TYPE_ABSTRACT = 2,
		TYPE_INTERFACE = 3,
		TYPE_MIXIN = 4,
	}
}
-- Weak values to let the garbage collector do it's thing correctly
setmetatable( m.instanceRegistry.singletons, {__mode = "v"} )

local utils
utils = {
	trimString = function( str )
		return string.match( str, "^%s*(.-)%s*$" )
	end,

	splitString = function( str, sep )
		local ret, pos = {}, 1
		for i = 1, string.len( str ) do
			local front, back = string.find( str, sep, pos, true )
			if not front then break end
			ret[i] = utils.trimString( string.sub(str, pos, front -1) )
			pos = back +1
		end
		ret[#ret +1] = utils.trimString( string.sub(str, pos) )
		return ret
	end,

	emptyTable = function( tbl )
		for k, _ in pairs( tbl ) do
			tbl[k] = nil
		end
	end,

	copyTable = function( tblSrc, seen )
		local copy = {}
		setmetatable( copy, getmetatable(tblSrc) )
		for k, v in pairs( tblSrc ) do
			if type( v ) ~= "table" then
				copy[k] = v
			else
				seen = seen or {}
				seen[tblSrc] = copy
				if seen[v] then
					copy[k] = seen[v]
				else
					copy[k] = utils.copyTable( v, seen )
				end
			end
		end
		return copy
	end,

	mergeTable = function( tblSrc, tblDest )
		for k, v in pairs( tblSrc ) do
			if type( v ) == "table" and type( tblDest[k] ) == "table" then
				utils.mergeTable( v, tblDest[k] )
			else
				tblDest[k] = v
			end
		end
		return tblDest
	end,

	mergeTablePreserveDest = function( tblSrc, tblDest )
		for k, v in pairs( tblSrc ) do
			if type( v ) == "table" and type( tblDest[k] ) == "table" then
				utils.mergeTable( v, tblDest[k] )
			else
				if tblDest[k] == nil then -- Don't overwrite anything in dest, only add absent things
					tblDest[k] = v
				end
			end
		end
		return tblDest
	end,

	mergeTableErrorConflicts = function( tblSrc, tblDest, strErrFmt, fnErrCB )
		for k, v in pairs( tblSrc ) do
			if type( v ) == "table" and type( tblDest[k] ) == "table" then
				utils.mergeTable( v, tblDest[k], strErrFmt )
			else
				if tblDest[k] ~= nil then -- Error
					if fnErrCB then fnErrCB() end
					error( string.format(strErrFmt, k) )
				else
					tblDest[k] = v
				end
			end
		end
		return tblDest
	end,

	classGCRouter = function( pClassInstance )
		local meta = getmetatable(pClassInstance)
		if meta.__removed then
			return -- Lua is cleaning up fragmented userdata
		end

		local v = rawget( meta.__classDef.methods, "__GC" )
		if v then
			v( pClassInstance )
		end
		meta.__classDef:OnGC( pClassInstance )
	end,
}

if not is51 then
	function utils.setfenv( fn, env )
		local i = 1
		while true do
			local name = debug.getupvalue( fn, i )
			if name == "_ENV" then
				debug.upvaluejoin( fn, i, function()
					return env
				end, 1 )
				break
			elseif not name then
				break
			end
			i = i +1
		end
		return fn
	end

	function utils.getfenv( fn )
		local i = 1
		while true do
			local name, val = debug.getupvalue( fn, i )
			if name == "_ENV" then
				return val
			elseif not name then
				break
			end
			i = i +1
		end
	end
end

function m:GetOrCreateNamespace( strNamespace )
	local t = m.g
	for _, v in ipairs( utils.splitString(strNamespace, ".") ) do
		t[v] = t[v] or {}
		t = t[v]
	end
	return t
end


--[[
	Class declaration object
	Created by calls to class creation methods (ex: class(), abstract(), singleton(), interface())
]]--
do
	local classDecMT = {}
	local classDecMTDef = {
		__index = classDecMT,
		__call = function( dec, ... )
			return m:CreateClassDefinition( dec, ... )
		end,
	}

	-- A bit weird here, to allow for proper method chaining me may end up wanting to call methods like final()
	-- after 'extend "" : from ""', we can't easily reconcile this so we must act as a proxy type in such instances
	local extendMT = {
		from = function( self, strNamespace )
			self.dec:SetParentNamespace( strNamespace )
			return self
		end,
	}

	local extendMTObj = {
		__index = function( t, k )
			if k == "from" then
				return extendMT.from
			else
				return rawget( t, "dec" )[k]
			end
		end,
		__newindex = function( t, k, v )
			rawget( t, "dec" )[k] = v
		end,
		__call = function( t, ... )
			return classDecMTDef.__call( rawget(t, "dec"), ... )
		end,
	}

	-- Pretty much the same story as above with extends, except we must 'finalize' implements() in edge case
	-- (when from() is never called)
	local implementsMT = {
		from = function( self, strNamespace )
			self.ns = strNamespace
			return self
		end,
	}

	local implementsMTObj = {
		__index = function( t, k )
			if not rawget( t, "finalized" ) and k == "from" then
				return implementsMT.from
			else
				-- Finalize first
				if not rawget( t, "finalized" ) then
					rawget( t, "dec" ):AddInterface( rawget(t, "iface"), rawget(t, "ns") )
					rawset( t, "finalized", true )
				end

				return rawget( t, "dec" )[k]
			end
		end,
		__newindex = function( t, k, v )
			if k == "ns" then
				rawset( t, k, v )
			else
				rawget( t, "dec" )[k] = v
			end
		end,
		__call = function( t, ... )
			-- Finalize first
			if not rawget( t, "finalized" ) then
				rawget( t, "dec" ):AddInterface( rawget(t, "iface"), rawget(t, "ns") )
				rawset( t, "finalized", true )
			end

			return classDecMTDef.__call( rawget(t, "dec"), ... )
		end,
	}

	-- For mixins, more of the same
	--[[local mixinMT = {
		from = function( self, strNamespace )
			self.ns = strNamespace
			return self
		end,
	}]]--

	local mixinMTObj = {
		__index = function( t, k )
			if not rawget( t, "finalized" ) and k == "from" then
				return implementsMT.from
			else
				-- Finalize first
				if not rawget( t, "finalized" ) then
					rawget( t, "dec" ):AddMixin( rawget(t, "name"), rawget(t, "ns") )
					rawset( t, "finalized", true )
				end

				return rawget( t, "dec" )[k]
			end
		end,
		__newindex = function( t, k, v )
			if k == "ns" then
				rawset( t, k, v )
			else
				rawget( t, "dec" )[k] = v
			end
		end,
		__call = function( t, ... )
			-- Finalize first
			if not rawget( t, "finalized" ) then
				rawget( t, "dec" ):AddMixin( rawget(t, "name"), rawget(t, "ns") )
				rawset( t, "finalized", true )
			end

			return classDecMTDef.__call( rawget(t, "dec"), ... )
		end,
	}

	local classDecInst = {
		type = m.classTypes.TYPE_CLASS,
		className = nil,
		m_namespace = nil,
		parentClass = nil,
		parentNamespace = nil,
		m_final = false,
		interfaces = {},
		mixins = {},
	}

	function classDecMT:SetClassType( nClassType )
		self.type = nClassType
	end

	function classDecMT:SetClassName( strName )
		self.className = strName
	end

	function classDecMT:SetNamespace( strNamespace )
		self.m_namespace = strNamespace
	end

	function classDecMT:SetParentClass( strClass )
		self.parentClass = strClass
	end

	function classDecMT:SetParentNamespace( strNamespace )
		self.parentNamespace = strNamespace
	end

	function classDecMT:AddInterface( strInterface, strNamespace )
		table.insert( self.interfaces, {
			iface = strInterface,
			ns = strNamespace,
		} )
	end

	function classDecMT:AddMixin( strMixin, strNamespace )
		local name = (strNamespace and (strNamespace.. ".") or "").. strMixin
		if self.mixins[name] then
			error( string.format("Attempt to add a mixin that has already been added! (Mixin: %s)\n", name) )
			return
		end
		self.mixins[name] = { name = strMixin, namespace = strNamespace, path = name }
	end

	function classDecMT:extends( strClassName )
		if self.type == m.classTypes.TYPE_INTERFACE then
			error( "Extending an interface is not allowed.\n" )
			return
		elseif self.type == m.classTypes.TYPE_MIXIN then
			error( "Extending a mixin is not allowed.\n" )
			return
		end

		self:SetParentClass( strClassName )

		local extendObj = { dec = self }
		setmetatable( extendObj, extendMTObj )
		return extendObj
	end

	function classDecMT:namespace( strNamespace )
		self:SetNamespace( strNamespace )
		return self
	end

	function classDecMT:final()
		if self.type == m.classTypes.TYPE_INTERFACE then
			error( "Marking an interface as final has no meaning, this is not allowed.\n" )
			return
		elseif self.type == m.classTypes.TYPE_MIXIN then
			error( "Marking a mixin as final has no meaning, this is not allowed.\n" )
			return
		end

		self.m_final = true
		return self
	end

	function classDecMT:implements( strInterface )
		if self.type == m.classTypes.TYPE_INTERFACE then
			error( "An interface may not implement another interface.\n" )
			return
		elseif self.type == m.classTypes.TYPE_MIXIN then
			error( "A mixin may not implement an interface.\n" )
			return
		end

		local implementsObj = { dec = self, iface = strInterface }
		setmetatable( implementsObj, implementsMTObj )
		return implementsObj
	end

	function classDecMT:mixin( strMixin )
		if self.type == m.classTypes.TYPE_INTERFACE then
			error( "Applying mixins to an interface is not allowed.\n" )
			return
		elseif self.type == m.classTypes.TYPE_MIXIN then
			error( "Applying mixins to another mixin is not allowed.\n" )
			return
		end

		local mixinObj = { dec = self, name = strMixin }
		setmetatable( mixinObj, mixinMTObj )
		return mixinObj
	end

	function m:CreateClassDeclaration()
		local inst = utils.copyTable( classDecInst )
		setmetatable( inst, classDecMTDef )
		return inst
	end
end


--[[
	Class definition object
	Created by calls to class declaration objects (declaring members at the end of the call chain via {})
]]--
do
	local classDefMT = {}
	local classDefMTDef = {
		__index = classDefMT,
	}
	local classDefInst = {
		declaration = nil,
		classID = 0,

		refCount = 0,

		-- namespace and parent info
		namespace = nil,
		parent = nil,
		parentNamespace = nil,

		instanceProto = {}, -- Final instance prototype

		-- member data
		members = {}, -- Normal member vars that are copied to all new instances

		-- method data
		methods = {},

		-- interface info
		interfaces = {},

		-- mixins
		mixins = {
			list = {},
			methods = {},
			members = {},
		},

		-- shared data
		shared_proto = nil,
		shared_instance = nil,

		-- meta methods
		metaMethods = {},
		meta = nil,

		finalized = false,
	}

	local metaLookup = {
		["__tostring"] 	= true,
		["__call"]  	= true,
		["__eq"] 		= true,
		["__unm"]		= true,
		["__add"]		= true,
		["__sub"]		= true,
		["__mul"]		= true,
		["__div"]		= true,
		["__pow"]		= true,
		["__concat"]	= true,
		["__lt"] 		= true,
		["__le"] 		= true,
		["__len"] 		= true,
	}

	local removedMT = {
		__index = function() error( "Attempt to index a removed class!\n" ) end,
		__newindex = function() error( "Attempt to index a removed class!\n" ) end,
		__removed = true,
	}

	function classDefMT:GetMembers()
		return self.members
	end

	function classDefMT:GetMethods()
		return self.methods
	end

	function classDefMT:GetMixins()
		return self.mixins
	end

	function classDefMT:GetParent()
		return self.parent
	end

	function classDefMT:GetNamespace()
		return self.namespace
	end

	function classDefMT:GetParentNamespace()
		return self.parentNamespace
	end

	function classDefMT:GetClassDeclaration()
		return self.declaration
	end

	function classDefMT:GetInstances()
		if self.declaration.type ~= m.classTypes.TYPE_CLASS then return end
		return m.instanceRegistry.classes[self.classID] and m.instanceRegistry.classes[self.classID] or {}
	end

	function classDefMT:FindSuperMethod( pInstance, strFunc )
		local cur = pInstance
		while type( cur.parent ) == "table" do
			if type( cur.parent.methods[strFunc] ) == "function" then
				return cur.super.methods[strFunc]
			end
			if cur == cur.parent then break end
			cur = cur.parent
		end
		return
	end

	function classDefMT:CallConstructor( pInstance, ... )
		if type( pInstance.Initialize ) == "function" then
			pInstance:Initialize( ... )
		else
			local method = self:FindSuperMethod( pInstance, "Initialize" )
			if not method then error( string.format("Class %s is missing a constructor.\n", self.declaration.className) )
				return
			end
			method( pInstance, ... )
		end

		return pInstance
	end

	function classDefMT:IncRefCount()
		self.refCount = self.refCount +1
		if self.refCount == 1 then
			-- Setup shared instance data
			if self.shared_proto then
				utils.emptyTable( self.shared_instance )
				self.shared_instance = utils.mergeTable( self.shared_proto, self.shared_instance )
			end
		end
	end

	function classDefMT:DecRefCount()
		assert( self.refCount > 0, "Class destructor error!\n" )
		self.refCount = self.refCount -1
		if self.refCount == 0 then
			-- Clear shared instance data
			if self.shared_proto and self.shared_instance then
				utils.emptyTable( self.shared_instance )
			end
		end
	end

	local RemoveInstance
	do
		local def, t, meta
		RemoveInstance = function( pClassInstance )
			meta = getmetatable( pClassInstance )
			if meta.__removed then return end

			def = meta.__classDef
			if def.declaration.type == m.classTypes.TYPE_SINGLETON then
				m.instanceRegistry.singletons[def.classID] = nil
			else
				t = m.instanceRegistry.classes[def.classID]
				if t then
					t[pClassInstance] = nil
				end
			end

			if meta.__removed then return end
			if type( pClassInstance.OnRemove ) == "function" then
				pClassInstance:OnRemove()
			end

			meta.__index = removedMT.__index
			meta.__newindex = removedMT.__newindex
			meta.__removed = true
			meta.__gc = nil
			meta.__tostring = nil
			meta.__call = nil
			meta.__eq = nil
			meta.__unm = nil
			meta.__add = nil
			meta.__sub = nil
			meta.__mul = nil
			meta.__div = nil
			meta.__pow = nil
			meta.__concat = nil
			meta.__lt = nil
			meta.__le = nil
			meta.__len = nil
		end
	end

	do
		local p
		function classDefMT:OnGC( pClassInstance )
			self:DecRefCount()
			p = self.parent
			while p ~= nil do
				p:DecRefCount()
				p = p.parent
			end

			RemoveInstance( pClassInstance )
		end
	end

	function classDefMT:New( ... )
		if self.declaration.type == m.classTypes.TYPE_SINGLETON then
			if m.instanceRegistry.singletons[self.classID] then
				-- If we already have a singleton instance, return that instance
				return m.instanceRegistry.singletons[self.classID]
			end
		end

		if self.declaration.type == m.classTypes.TYPE_ABSTRACT or
			self.declaration.type == m.classTypes.TYPE_INTERFACE or
			self.declaration.type == m.classTypes.TYPE_MIXIN then
			error( "Cannot create an instance of an abstract, interface or mixin class.\n" )
		end

		-- Construct the instance
		local meta = {
			__gc = self.meta.__gc,
			__tostring = self.meta.__tostring,
			__call = self.meta.__call,
			__eq = self.meta.__eq,
			__unm = self.meta.__unm,
			__add = self.meta.__add,
			__sub = self.meta.__sub,
			__mul = self.meta.__mul,
			__div = self.meta.__div,
			__pow = self.meta.__pow,
			__concat = self.meta.__concat,
			__lt = self.meta.__lt,
			__le = self.meta.__le,
			__len = self.meta.__len,
			__classDef = self.meta.__classDef,
		}
		meta.__mt = meta

		local instance
		if is51 then
			-- Make a userdata proxy to get __gc functionality
			local realInstance = utils.copyTable( self.instanceProto ) -- copy member vars into the instance

			instance = newproxy( true )
			local mt = getmetatable( instance )
			mt.__instance = realInstance -- Hard ref to keep the instance alive for the lifetime of the proxy
			mt.__index = realInstance
			mt.__newindex = realInstance
			mt.__gc = meta.__gc
			mt.__tostring = meta.__tostring
			mt.__call = meta.__call
			mt.__eq = meta.__eq
			mt.__unm = meta.__unm
			mt.__add = meta.__add
			mt.__sub = meta.__sub
			mt.__mul = meta.__mul
			mt.__div = meta.__div
			mt.__pow = meta.__pow
			mt.__concat = meta.__concat
			mt.__lt = meta.__lt
			mt.__le = meta.__le
			mt.__len = meta.__len
			mt.__classDef = meta.__classDef
			mt.__mt = mt
		else
			local realInstance = utils.copyTable( self.instanceProto ) -- copy member vars into the instance
			-- we need a proxy table due to metatable changes in 5.2+ (__index only being invoked when a var isn't found)
			instance = {}
			local mt = {
				__instance = realInstance,
				__index = realInstance,
				__newindex = realInstance,
				__gc = meta.__gc,
				__tostring = meta.__tostring,
				__call = meta.__call,
				__eq = meta.__eq,
				__unm = meta.__unm,
				__add = meta.__add,
				__sub = meta.__sub,
				__mul = meta.__mul,
				__div = meta.__div,
				__pow = meta.__pow,
				__concat = meta.__concat,
				__lt = meta.__lt,
				__le = meta.__le,
				__len = meta.__len,
				__classDef = meta.__classDef,
			}
			mt.__mt = mt
			setmetatable( instance, mt )
		end

		self:IncRefCount()
		local p = self.parent
		while p ~= nil do
			p:IncRefCount()
			p = p.parent
		end

		-- If we are a singleton, add to the singleton list
		if self.declaration.type == m.classTypes.TYPE_SINGLETON then
			m.instanceRegistry.singletons[self.classID] = instance
		else
			-- Add the class to the instance registry
			if not m.instanceRegistry.classes[self.classID] then
				m.instanceRegistry.classes[self.classID] = {}
				--Weak values to let the garbage collector do it's thing correctly
				setmetatable( m.instanceRegistry.classes[self.classID], {__mode = "kv"} )
			end
			m.instanceRegistry.classes[self.classID][instance] = instance
		end

		return self:CallConstructor( instance, ... )
	end

	function classDefMT:FinalizeMixins()
		for _, v in pairs( self.declaration.mixins ) do
			local ns = m.g
			if v.namespace then
				ns = m:GetOrCreateNamespace( v.namespace )
			end

			local mixin = ns[v.name]
			if not mixin then
				m:CleanupFailedDef( self )
				error( string.format("Unable to find mixin '%s'\n", (v.namespace and (v.namespace.. ".") or "").. v.name) )
				return
			end

			self.mixins.list[v.path] = mixin
			self.mixins.members = utils.mergeTableErrorConflicts(
				mixin.members,
				self.mixins.members,
				"Mixin member conflict! (%s)\n",
				function()
					m:CleanupFailedDef( self )
				end
			)
			self.mixins.methods = utils.mergeTableErrorConflicts(
				mixin.methods,
				self.mixins.methods,
				"Mixin method conflict! (%s)\n",
				function()
					m:CleanupFailedDef( self )
				end
			)
		end

		-- Parent mixins
		if self.parent then
			for _, v in pairs( self.parent.declaration.mixins ) do
				if self.mixins.list[v.path] then
					local cns = self.declaration.m_namespace
					cns = cns and (cns.. ".") or ""

					m:CleanupFailedDef( self )
					error( string.format(
						"Parent class '%s' contains the same mixin as the child class '%s' (mixin '%s')\n",
						self.parent.declaration.className,
						cns.. self.declaration.className,
						(v.namespace and (v.namespace.. ".") or "").. v.name
					) )
				end
			end
		end

		-- Merge mixins with class data
		self.members = utils.mergeTableErrorConflicts(
			self.mixins.members,
			self.members,
			"Mixin member conflict! (%s)\n",
			function()
				m:CleanupFailedDef( self )
			end
		)
		self.methods = utils.mergeTableErrorConflicts(
			self.mixins.methods,
			self.methods,
			"Mixin method conflict! (%s)\n",
			function()
				m:CleanupFailedDef( self )
			end
		)
	end

	function classDefMT:FinalizeInterfaces()
		-- Merge parent interfaces
		if self.parent then
			for id, iface in pairs( self.parent.interfaces ) do
				local dec = m.classRegistry[iface.classID].declaration
				if self.interfaces[id] then
					local cns = self.declaration.m_namespace
					cns = cns and (cns.. ".") or ""

					local ins = dec.m_namespace
					ins = ins and (ins.. ".") or ""

					m:CleanupFailedDef( self )
					error( string.format(
						"Interface re-implementation in class '%s' from parent '%s' - interface '%s'",
						cns.. self.declaration.className,
						self.parent.declaration.className,
						ins.. dec.className
					) )
					return
				end

				self.interfaces[id] = iface
			end
		end

		-- Check that interfaces are all implemented
		for _, iface in pairs( self.interfaces ) do
			for k, v in pairs( iface.methods ) do
				if type( self.methods[k] ) ~= "function" or self.methods[k] == v then
					local dec = m.classRegistry[iface.classID].declaration
					m:CleanupFailedDef( self )
					error( string.format(
						"Class '%s' uses interface '%s' but does not implement method '%s'!\n",
						(self.declaration.m_namespace and (self.declaration.m_namespace.. ".") or "").. self.declaration.className,
						dec.className,
						k
					) )
				end
			end
		end
	end

	function classDefMT:Finalize()
		-- Register mixins
		self:FinalizeMixins()
		-- Interfaces
		self:FinalizeInterfaces()
		-- Inherit metamethods if we have any
		if self.parent then
			for name, _ in pairs( metaLookup ) do
				if self.parent.metaMethods[name] then
					if not self.metaMethods[name] then
						self.metaMethods[name] = self.parent.metaMethods[name]
					end
				end
			end
		end

		-- Build our metatable
		self.meta = {
			__gc = utils.classGCRouter,
			__tostring = self.metaMethods.__tostring,
			__call = self.metaMethods.__call,
			__eq = self.metaMethods.__eq,
			__unm = self.metaMethods.__unm,
			__add = self.metaMethods.__add,
			__sub = self.metaMethods.__sub,
			__mul = self.metaMethods.__mul,
			__div = self.metaMethods.__div,
			__pow = self.metaMethods.__pow,
			__concat = self.metaMethods.__concat,
			__lt = self.metaMethods.__lt,
			__le = self.metaMethods.__le,
			__len = self.metaMethods.__len,
			__classDef = self,
		}

		-- Build the instance prototype
		self.instanceProto = utils.copyTable( self.members )
		self.instanceProto = utils.mergeTableErrorConflicts(
			self.methods,
			self.instanceProto,
			"Method/member name conflict!\n",
			function()
				m:CleanupFailedDef( self )
			end
		)

		if not self.instanceProto.Remove then
			self.instanceProto.Remove = RemoveInstance
		end

		m.classRegistry[self.classID] = self
		self.finalized = true

		if self.finalizer then
			self.finalizer()
		end
	end

	function m:CleanupFailedDef( pClassDef )
		pClassDef.namespace[pClassDef.declaration.className] = nil
		m.classRegistry[pClassDef.classID] = nil
		m.classIndex = m.classIndex -1
	end

	function m:CreateClassDefinition( pClassDec, tblMembers )
		-- Get the namespace
		local ns = m.g
		if pClassDec.m_namespace then
			ns = self:GetOrCreateNamespace( pClassDec.m_namespace )
		end

		-- Create a new class def
		local def = utils.copyTable( classDefInst )
		setmetatable( def, classDefMTDef )

		-- set declaration and namespace
		def.classID = m.classIndex; m.classIndex = m.classIndex +1
		def.declaration = pClassDec
		def.namespace = ns

		-- Get the parent namespace (if we have a parent)
		if pClassDec.parentClass then
			pClassDec.parentNamespace = pClassDec.parentNamespace or pClassDec.m_namespace

			if pClassDec.parentClass == pClassDec.className and pClassDec.parentNamespace == pClassDec.m_namespace then
				m:CleanupFailedDef( def )
				error( string.format(
					"Class %s is trying to extend itself!\n",
					(pClassDec.m_namespace and (pClassDec.m_namespace.. ".") or "").. pClassDec.className
				) )
				return
			end

			local parentNS = ns
			if pClassDec.parentNamespace then
				parentNS = self:GetOrCreateNamespace( pClassDec.parentNamespace )
			end

			-- Set parent info
			def.parent = parentNS[pClassDec.parentClass]
			def.parentNamespace = parentNS

			if not def.parent then
				m:CleanupFailedDef( def )
				error( string.format(
					"Parent class '%s' is nil - child class '%s'!\n",
					pClassDec.parentClass,
					(pClassDec.m_namespace and (pClassDec.m_namespace.. ".") or "").. pClassDec.className
				) )
				return
			end

			-- Make sure we aren't trying to extend from a final class
			if def.parent.declaration.m_final then
				m:CleanupFailedDef( def )
				error( string.format(
					"Attempt to extend the class '%s' is not allowed, '%s' is marked as final.\n",
					pClassDec.parentClass, pClassDec.parentClass
				) )
			end

			-- Mix the parent members with our class
			tblMembers = utils.mergeTablePreserveDest( parentNS[pClassDec.parentClass]:GetMembers(), tblMembers )
			-- Add parent methods
			def.methods = utils.mergeTablePreserveDest( parentNS[pClassDec.parentClass]:GetMethods(), def.methods )
		end

		-- Lookup interfaces
		if #pClassDec.interfaces > 0 then
			for _, i in pairs( pClassDec.interfaces ) do
				local namespace = i.ns and i.ns or pClassDec.m_namespace
				local ins = namespace and self:GetOrCreateNamespace( namespace ) or m.g
				local iface = ins[i.iface]
				def.interfaces[iface.classID] = iface
			end
		end

		-- Set the members
		def.members = tblMembers

		-- Set the definition in the class namespace
		if def.namespace[pClassDec.className] ~= nil then
			m:CleanupFailedDef( def )
			error( string.format("Class name conflict! Found another variable at '%s' ('%s')\n",
				(pClassDec.m_namespace and (pClassDec.m_namespace.. ".") or "").. pClassDec.className,
				tostring( def.namespace[pClassDec.className] )
			) )
			return
		end
		def.namespace[pClassDec.className] = def

		m:HookClassEnv( def )
	end

	local function DefineAccessorFunc( pObjDef, str )
		local data = utils.splitString( str, "->" )
		local name, var = data[1], data[2]
		pObjDef["Set".. name] = function( s, val )
			s[var] = val
		end
		pObjDef["Get".. name] = function( s )
			return s[var]
		end
	end

	local function DefineGetterFunc( pObjDef, str )
		local data = utils.splitString( str, "->" )
		local name, var = data[1], data[2]
		pObjDef["Get".. name] = function( s )
			return s[var]
		end
		return pObjDef["Get".. name]
	end

	local function DefineSetterFunc( pObjDef, str )
		local data = utils.splitString( str, "->" )
		local name, var = data[1], data[2]
		pObjDef["Set".. name] = function( s, val )
			s[var] = val
		end
		return pObjDef["Set".. name]
	end

	local function findMainChunk()
		local i = 2
		while true do
			local info = debug.getinfo( i, "nS" )
			if not info then break end
			if info.what == "main" then
				return i -1
			end
			i = i +1
		end
	end

	function m:HookClassEnv( pClassDef )
		local index = 3 -- For LuaJIT, this is always stack position 3
		if is51 and not isJit then
			-- Lua 5.1 will contain extra tail calls in some cases
			-- we need to find the main chunk
			index = findMainChunk()
			if not index then
				m:CleanupFailedDef( pClassDef )
				error( "Unable to locate class environment!\n" )
			end
		end

		-- If we are on Lua 5.2 or later, we use _ENV instead
		local mt = is51 and getmetatable( getfenv(index) ) or getmetatable( m.__ACTIVE_CLASS_ENV )
		if mt and rawget( mt, "__isAClass" ) then
			m:CleanupFailedDef( pClassDef )
			error( "Class definitions are restricted to 1 class per file/function!\n" )
			return
		end

		m.__ACTIVE_CLASS_DEF = pClassDef

		local lookup
		lookup = {
			this = pClassDef,
			super = pClassDef.parent and pClassDef.parent.methods or nil,
			super_shared = pClassDef.parent and pClassDef.parent.shared_instance or nil,
			accessor = function( str )
				DefineAccessorFunc( pClassDef.methods, str )
			end,
			getter = function( str )
				DefineGetterFunc( pClassDef.methods, str )
			end,
			setter = function( str )
				DefineSetterFunc( pClassDef.methods, str )
			end,
			shared_block = function( tblData )
				if pClassDef.finalized then
					error( "A shared_block can only be defined during class definition!\n" )
					return
				end

				if pClassDef.shared_proto ~= nil then
					m:CleanupFailedDef( pClassDef )
					error( "A class definition may only contain 1 shared block!\n" )
					return
				end
				pClassDef.shared_proto = tblData
				pClassDef.shared_instance = {}
				lookup.shared = pClassDef.shared_instance
			end,
			finally = function( fnCB )
				if pClassDef.finalizer then
					error( "A class definition may only specify one finalizer!\n" )
					return
				end

				pClassDef.finalizer = fnCB
			end,
		}

		local envMeta
		local __index = function( _, k )
			local v = lookup[k]
			return v and v or rawget( m.g, k )
		end
		local __newindex = function( t, k, v )
			if type( v ) == "function" and t == envMeta then
				if metaLookup[k] then
					rawset( pClassDef.metaMethods, k, v )
				else
					rawset( pClassDef.methods, k, v )
				end
			else
				if t ~= envMeta then
					rawset( t, k, v )
				else
					rawset( m.g, k, v )
				end
			end
		end

		if is51 then
			envMeta = setmetatable( {}, {
				__index = __index,
				__newindex = __newindex,
				__isAClass = true,
			} )
			setfenv( index, envMeta )
		else
			mt.__index = __index
			mt.__newindex = __newindex
			mt.__isAClass = true
			envMeta = setmetatable( m.__ACTIVE_CLASS_ENV, mt )
		end
	end
end


return {
	abstract = function( strName )
		local dec = m:CreateClassDeclaration()
		dec:SetClassType( m.classTypes.TYPE_ABSTRACT )
		dec:SetClassName( strName )
		return dec
	end,

	singleton = function( strName )
		local dec = m:CreateClassDeclaration()
		dec:SetClassType( m.classTypes.TYPE_SINGLETON )
		dec:SetClassName( strName )
		return dec
	end,

	class = function( strName )
		local dec = m:CreateClassDeclaration()
		dec:SetClassType( m.classTypes.TYPE_CLASS )
		dec:SetClassName( strName )
		return dec
	end,

	interface = function( strName )
		local dec = m:CreateClassDeclaration()
		dec:SetClassType( m.classTypes.TYPE_INTERFACE )
		dec:SetClassName( strName )
		return dec
	end,

	mixin = function( strName )
		local dec = m:CreateClassDeclaration()
		dec:SetClassType( m.classTypes.TYPE_MIXIN )
		dec:SetClassName( strName )
		return dec
	end,

	implements = function( pClassInstance, pClassDef )
		return getmetatable( pClassInstance ).__classDef.interfaces[pClassDef.classID] and true or false
	end,

	is_a = function( pClassInstance, pClassDef )
		return getmetatable( pClassInstance ).__classDef and
			getmetatable( pClassInstance ).__classDef.classID == pClassDef.classID
	end,

	validClass = is51 and
		(function( pClassInstance )
			return type( pClassInstance ) == "userdata" and getmetatable(pClassInstance).__classDef ~= nil
		end)
	or
		(function( pClassInstance )
			return type( pClassInstance ) == "table" and getmetatable(pClassInstance).__classDef ~= nil
		end),

	loadClass = function( strFile )
		if is51 then
			-- Lua 5.1/JIT will setfenv the file scope
			dofile( strFile )
			m.__ACTIVE_CLASS_DEF:Finalize()
			m.__ACTIVE_CLASS_DEF = nil
		else
			-- Lua 5.2+ must set an _ENV metatable and grab a reference to it
			m.__ACTIVE_CLASS_ENV = setmetatable( {}, {__index = m.g} )
			m.__ACTIVE_CLASS_CHUNK = loadfile( strFile, nil, m.__ACTIVE_CLASS_ENV )
			m.__ACTIVE_CLASS_CHUNK()
			m.__ACTIVE_CLASS_DEF:Finalize()
			m.__ACTIVE_CLASS_DEF = nil
		end
	end,

	removeClass = function( pClassDef )
		m.classRegistry[pClassDef.classID] = nil
		if pClassDef:GetInstances() then
			for _, v in pairs( pClassDef:GetInstances() ) do
				v:Remove()
			end
		end
		m.instanceRegistry.classes[pClassDef.classID] = nil
		m.instanceRegistry.singletons[pClassDef.classID] = nil
		pClassDef.namespace[pClassDef.declaration.className] = nil

		if pClassDef.declaration.type == m.classTypes.TYPE_SINGLETON then
			pClassDef:Remove()
		end
	end,
}