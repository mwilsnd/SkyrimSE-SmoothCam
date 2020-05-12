#pragma once
#include "versiondb.h"
#include "offsets.h"
#include "skse64_common/Relocation.h"

// A custom member fn impl that translates 1.5.97 offsets to address IDs


// this has been tested to work for non-varargs functions
// varargs functions end up with 'this' passed as the last parameter (ie. probably broken)
// do NOT use with classes that have multiple inheritance

// if many member functions are to be declared, use MEMBER_FN_PREFIX to create a type with a known name
// so it doesn't need to be restated throughout the member list

// all of the weirdness with the _GetType function is because you can't declare a static const pointer
// inside the class definition. we sadly can't inline anymore because of relocation.

// RelocPtr only works at a global scope, which we can't handle or we'd be bypassing the function route altogether

// This construct forces a compiler error when we haven't defined an address in our map
template<uintptr_t N>
struct AddrNotInMapWarning {
	uintptr_t adr = N;
};

#define MEMBER_FN_PREFIX(className)	\
	typedef className _MEMBER_FN_BASE_TYPE

#define DEFINE_MEMBER_FN_LONG(className, functionName, retnType, address, ...)								\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);									\
	static constexpr const AddrNotInMapWarning<Offsets::addrMap.at(address)> _##functionName##_adr = {};	\
	inline _##functionName##_type * _##functionName##_GetPtr(void)											\
	{																										\
		static const uintptr_t adr = Offsets::GetVersionAddress(address) + RelocationManager::s_baseAddr;	\
		return (_##functionName##_type *)&adr;																\
	}

#define DEFINE_MEMBER_FN(functionName, retnType, address, ...)	\
	DEFINE_MEMBER_FN_LONG(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, __VA_ARGS__)

#define DEFINE_STATIC_HEAP(staticAllocate, staticFree)						\
	static void * operator new(std::size_t size)							\
	{																		\
		return staticAllocate(size);										\
	}																		\
	static void * operator new(std::size_t size, const std::nothrow_t &)	\
	{																		\
		return staticAllocate(size);										\
	}																		\
	static void * operator new(std::size_t size, void * ptr)				\
	{																		\
		return ptr;															\
	}																		\
	static void operator delete(void * ptr)									\
	{																		\
		staticFree(ptr);													\
	}																		\
	static void operator delete(void * ptr, const std::nothrow_t &)			\
	{																		\
		staticFree(ptr);													\
	}																		\
	static void operator delete(void *, void *)								\
	{																		\
	}

#define CALL_MEMBER_FN(obj, fn)	\
	((*(obj)).*(*((obj)->_##fn##_GetPtr())))

// Using the original implementation does very broken things in a Release build
#define FORCE_INLINE  __forceinline
#define DEFINE_MEMBER_FN_0(fnName, retnType, addr)						\
	FORCE_INLINE retnType fnName() {										\
	struct empty_struct {};													\
	typedef retnType(empty_struct::*_##fnName##_type)();					\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;	\
	_##fnName##_type fn = *(_##fnName##_type*)&address;						\
	return (reinterpret_cast<empty_struct*>(this)->*fn)();					\
	}
#define DEFINE_MEMBER_FN_1(fnName, retnType, addr, ...)					\
	template<typename T1>													\
	FORCE_INLINE retnType fnName(T1 && t1) {								\
	struct empty_struct {};													\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);			\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;	\
	_##fnName##_type fn = *(_##fnName##_type*)&address;						\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1);				\
	}
#define DEFINE_MEMBER_FN_2(fnName, retnType, addr, ...)					\
	template<typename T1, typename T2>										\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2) {						\
	struct empty_struct {};													\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);			\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;	\
	_##fnName##_type fn = *(_##fnName##_type*)&address;						\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2);			\
	}
#define DEFINE_MEMBER_FN_3(fnName, retnType, addr, ...)					\
	template<typename T1, typename T2, typename T3>							\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3) {			\
	struct empty_struct {};													\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);			\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;	\
	_##fnName##_type fn = *(_##fnName##_type*)&address;						\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3);		\
	}
#define DEFINE_MEMBER_FN_4(fnName, retnType, addr, ...)					\
	template<typename T1, typename T2, typename T3, typename T4>			\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4) {	\
	struct empty_struct {};													\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);			\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;	\
	_##fnName##_type fn = *(_##fnName##_type*)&address;						\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4);	\
	}
#define DEFINE_MEMBER_FN_5(fnName, retnType, addr, ...)								\
	template<typename T1, typename T2, typename T3, typename T4, typename T5>			\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5) {	\
	struct empty_struct {};																\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);						\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;				\
	_##fnName##_type fn = *(_##fnName##_type*)&address;									\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5);			\
	}
#define DEFINE_MEMBER_FN_6(fnName, retnType, addr, ...)										\
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>		\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6) {	\
	struct empty_struct {};																		\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);								\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;						\
	_##fnName##_type fn = *(_##fnName##_type*)&address;											\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5, t6);				\
	}
#define DEFINE_MEMBER_FN_7(fnName, retnType, addr, ...)													\
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>		\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7) {	\
	struct empty_struct {};																					\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);											\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;									\
	_##fnName##_type fn = *(_##fnName##_type*)&address;														\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5, t6, t7);						\
	}
#define DEFINE_MEMBER_FN_8(fnName, retnType, addr, ...)																\
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>	\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8) {		\
	struct empty_struct {};																								\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);														\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;												\
	_##fnName##_type fn = *(_##fnName##_type*)&address;																	\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5, t6, t7, t8);								\
	}
#define DEFINE_MEMBER_FN_9(fnName, retnType, addr, ...)																			\
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>	\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8, T9 && t9) {		\
	struct empty_struct {};																											\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);																	\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;															\
	_##fnName##_type fn = *(_##fnName##_type*)&address;																				\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5, t6, t7, t8, t9);										\
	}
#define DEFINE_MEMBER_FN_10(fnName, retnType, addr, ...)																						\
	template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>	\
	FORCE_INLINE retnType fnName(T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8, T9 && t9, T10 && t10) {		\
	struct empty_struct {};																														\
	typedef retnType(empty_struct::*_##fnName##_type)(__VA_ARGS__);																				\
	const static uintptr_t address = addr + RelocationManager::s_baseAddr;																		\
	_##fnName##_type fn = *(_##fnName##_type*)&address;																							\
	return (reinterpret_cast<empty_struct*>(this)->*fn)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10);												\
	}
