#ifndef MEMORYUTIL_H
#define MEMORYUTIL_H

#include <memory>
#include <cassert>
#include <d3d12.h>

struct ComDeleter
{
	template<typename T>
	void operator()(T* type)
	{
		type->Release();
	}
};


template <typename T, typename Deleter = std::default_delete<T>>
struct  unique_ptr : public std::unique_ptr<T, Deleter>
{
	static_assert(std::is_empty<Deleter>::value, "unique_ptr doesn't support stateful deleter.");
	typedef std::unique_ptr<T, Deleter> parent_t;
	using pointer = typename parent_t::pointer;

	unique_ptr() : parent_t(nullptr) {}
	template <class _Dx2 = Deleter, std::_Unique_ptr_enable_default_t<_Dx2> = 0>
	constexpr unique_ptr(nullptr_t) noexcept : parent_t(nullptr) {}

	explicit unique_ptr(T* p) : parent_t(p){}

	template <typename Del2>
	unique_ptr(unique_ptr<T, Del2>&& other)
		: parent_t(other.release())
	{
	}

	template <typename Del2>
	unique_ptr& operator=(unique_ptr<T, Del2>&& other)
	{
		parent_t::reset(other.release());
		return *this;
	}

	unique_ptr& operator=(pointer p)
	{
		reset(p);
		return *this;
	}

	void reset(pointer p = pointer())
	{
		parent_t::reset(p);
	}

	void reset(std::nullptr_t p)
	{
		parent_t::reset(p);
	}

	T** operator&()
	{
		assert(*this == nullptr);
		return reinterpret_cast<T**>(this);
	}

	T* const* operator&() const
	{
		return reinterpret_cast<T* const*>(this);
	}

	using parent_t::release;
	using parent_t::get;
	using parent_t::operator->;
	//using parent_t::operator*;
	using parent_t::operator bool;
	using parent_t::operator=;

private:

};


//template <typename T, typename Deleter = std::conditional_t<std::is_base_of<IUnknown, T>::value, CustomDeleter, std::default_delete<T>>>
//struct  unique_ptr : public std::unique_ptr<T, ComDeleter>

// for ComPtr
template <typename T>  requires std::derived_from<T, IUnknown>
struct  unique_ptr<T> : public std::unique_ptr<T, ComDeleter>
{
	typedef std::unique_ptr<T, ComDeleter> parent_t;
	using pointer = typename parent_t::pointer;

	unique_ptr() : parent_t(nullptr) {}
	template <class _Dx2 = ComDeleter, std::_Unique_ptr_enable_default_t<_Dx2> = 0>
	constexpr unique_ptr(nullptr_t) noexcept : parent_t(nullptr) {}

	template <class _Dx2 = ComDeleter, std::_Unique_ptr_enable_default_t<_Dx2> = 0>
	explicit unique_ptr(pointer _Ptr) noexcept : parent_t(_Ptr)
	{
		if (_Ptr)
		{
			_Ptr->AddRef();
		}
	}


	explicit unique_ptr(T* _Ptr) : parent_t(_Ptr)
	{
		if (_Ptr)
		{
			_Ptr->AddRef();
		}
	}

	template <typename Del2>
	unique_ptr(unique_ptr<T, Del2>&& other)
		: parent_t(other.release())
	{
	}

	template <typename Del2>
	unique_ptr& operator=(unique_ptr<T, Del2>&& other)
	{
		parent_t::reset(other.release());
		return *this;
	}

	unique_ptr& operator=(pointer p)
	{
		reset(p);
		return *this;
	}

	void reset(pointer p = pointer())
	{
		if (p)
		{
			p->AddRef();
		}
		parent_t::reset(p);
	}

	void reset(std::nullptr_t p)
	{
		parent_t::reset(p);
	}

	T** operator&()
	{
		assert(*this == nullptr);
		return reinterpret_cast<T**>(this);
	}

	T* const* operator&() const
	{
		return reinterpret_cast<T* const*>(this);
	}

	using parent_t::release;
	using parent_t::get;
	using parent_t::operator->;
	//using parent_t::operator*;
	using parent_t::operator bool;
	using parent_t::operator=;

	/*template <class T, class _Dx2,
		std::enable_if_t<std::conjunction_v<std::negation<std::is_array<T>>, std::is_assignable<Deleter&, _Dx2>,
		std::is_convertible<typename unique_ptr<T, _Dx2>::pointer, pointer>>,
		int> = 0>

		unique_ptr& operator=(unique_ptr<T, _Dx2>&& _Right) noexcept {
		reset(_Right.release());
		_Mypair._Get_first() = std::forward<_Dx2>(_Right._Mypair._Get_first());
		return *this;
	}

	template <class _Dx2 = Deleter, std::enable_if_t<std::is_move_assignable_v<_Dx2>, int> = 0>
	unique_ptr& operator=(unique_ptr&& _Right) noexcept {
		if (this != std::addressof(_Right)) {
			reset(_Right.release());
			_Mypair._Get_first() = std::forward<Deleter>(_Right._Mypair._Get_first());
		}
		return *this;
	}*/
private:


};


template <class T, class... _Types >
inline unique_ptr<T> make_unique(_Types&&... _Args)  requires (!std::derived_from<T, IUnknown>&& !std::is_array_v<T>)
{
	return unique_ptr<T>(new T(std::forward<_Types>(_Args)...));
}

//use unique_ptr to create a pointer for ID3D12Object objects
template <class T>
inline unique_ptr<T> make_unique(const size_t _Size)  requires (!std::derived_from<T, IUnknown> && std::is_array_v<T>&& std::extent_v<T> == 0)
{ // make a unique_ptr
	using _Elem = std::remove_extent_t<T>;
	return unique_ptr<T>(new _Elem[_Size]());
}

#endif