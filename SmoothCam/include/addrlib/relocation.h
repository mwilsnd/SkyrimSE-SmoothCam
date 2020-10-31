#pragma once
#include "code_analysis.h"
SILENCE_CODE_ANALYSIS;
#   include "versiondb.h"
RESTORE_CODE_ANALYSIS;

#include "offsets.h"

// A custom relocation impl that translates 1.5.97 offsets to address IDs

class RelocationManager
{
	public:
	RelocationManager();

	static uintptr_t	s_baseAddr;
};

// use this for addresses that represent pointers to a type T
template <typename T>
class RelocPtr
{
	public:
	RelocPtr(uintptr_t offset)
		:m_offset(offset)
	{
		//
	}

	operator T *() const
	{
		return GetPtr();
	}

	T * operator->() const
	{
		return GetPtr();
	}

	T * GetPtr() const
	{
		static auto v = Offsets::GetVersionAddress<T*>(m_offset);
		return v;
	}

	const T * GetConst() const
	{
		static auto v = Offsets::GetVersionAddress<T*>(m_offset);
		return v;
	}

	uintptr_t GetUIntPtr() const
	{
		static auto v = Offsets::GetVersionAddress<uintptr_t>(m_offset);
		return v;
	}

	private:
	uintptr_t	m_offset;

	// hide
	RelocPtr();
	RelocPtr(RelocPtr & rhs);
	RelocPtr & operator=(RelocPtr & rhs);
};

// use this for direct addresses to types T. needed to avoid ambiguity
template <typename T>
class RelocAddr
{
	public:
	RelocAddr(uintptr_t offset)
		:m_offset(offset)
	{
		//
	}

	operator T()
	{
		static auto v = Offsets::GetVersionAddress<T>(m_offset);
		return v;
	}

	uintptr_t GetUIntPtr() const
	{
		static auto v = Offsets::GetVersionAddress<uintptr_t>(m_offset);
		return v;
	}

	private:
	uintptr_t m_offset;

	// hide
	RelocAddr();
	RelocAddr(RelocAddr & rhs);
	RelocAddr & operator=(RelocAddr & rhs);
};