#pragma once
#include <opencv2\opencv.hpp>
#include <memory>
#include <typeindex>

#define CHAOS_API 
/// <summary>
/// <para>Any对象，用于保存任意类型的变量</para>
/// </summary>
class Any
{
public:
	Any(void) : type_idx(std::type_index(typeid(void))) {}
	Any(const Any& that) : ptr(that.Create()), type_idx(that.type_idx) {}
	Any(Any && that) : ptr(std::move(that.ptr)), type_idx(that.type_idx) {}

	template<typename Type, class = typename std::enable_if<!std::is_same<typename std::decay<Type>::type, Any>::value, Type>::type> Any(Type && value) :
		ptr(new Derived<typename std::decay<Type>::type>(std::forward<Type>(value))), type_idx(std::type_index(typeid(typename std::decay<Type>::type))) {}

	/// <summary>判断Any类是否为空</summary>
	bool IsEmpty() const { return !bool(ptr); }

	/// <summary>判断Any的实际类型</summary>
	template<class Type> bool Is() const
	{
		return type_idx == std::type_index(typeid(Type));
	}

	/// <summary>向下转换Any到实际类型</summary>
	template<class Type>
	operator Type()
	{
		if (!Is<Type>())
		{
			std::cout << "Can not cast " << typeid(Type).name() << " to " << type_idx.name() << std::endl;
			throw std::bad_cast();
		}

		auto derived = dynamic_cast<Any::Derived<Type>*>(ptr.get());
		return derived->value;
	}

	/// <summary>复制</summary>
	Any& operator=(const Any& any)
	{
		if (ptr == any.ptr)
			return *this;

		ptr = any.Create();
		type_idx = any.type_idx;

		return *this;
	}

private:
	class Base
	{
	public:
		virtual std::unique_ptr<Base> Create() = 0;
	};

	template<typename DerivedType>
	class Derived : public Base
	{
	public:
		template<typename Type>
		Derived(Type && value) : value(std::forward<Type>(value)) {}
		std::unique_ptr<Base> Create()
		{
			return std::unique_ptr<Base>(new Derived<DerivedType>(value));
		}

		DerivedType value;
	};

	std::unique_ptr<Base> Create() const
	{
		if (ptr != nullptr)
			return ptr->Create();
		return nullptr;
	}

	std::unique_ptr<Base> ptr;
	std::type_index type_idx;
};

constexpr size_t prime = 0x100000001B3ull;
constexpr size_t basis = 0xCBF29CE484222325ull;
constexpr size_t Hash(const char* data, size_t value = basis)
{
	return *data ? Hash(data + 1, (*data ^ value) * prime) : value;
}
constexpr size_t operator "" _hash(const char* data, size_t)
{
	return Hash(data);
}