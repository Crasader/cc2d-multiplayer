#ifndef __DELEGATE_H__
#define __DELEGATE_H__

#include "ScriptFunction.h"

template <class ...Args>
struct Delegate
{
	using stub_type = void(*)(void*, void*, Args...);

	template <class T>
	struct MethodStub
	{
		typedef void (T::*FuncType)(Args...);

		static void invoke(void* objPtr, void* funcPtr, Args... params)
		{
			T* obj = static_cast<T*>(objPtr);
			FuncType func = *(FuncType*)&funcPtr;

			(obj->*func)(params...);
		}
	};

	struct FunctionStub
	{
		typedef void(*FuncType)(Args...);

		static void invoke(void* objPtr, void* funcPtr, Args... params)
		{
			FuncType func = *(FuncType*)&funcPtr;
			(*func)(params...);
		}
	};

	struct ScriptStub
	{
		typedef void(*FuncType)(Args...);

		static void invoke(void* objPtr, void* funcPtr, Args... params)
		{
			ScriptFunction* func = (ScriptFunction*)funcPtr;
			func->call(params...);
		}
	};

	void invoke(Args... params)
	{
		(*stubPtr)(objPtr, funcPtr, params...);
	}

	Delegate()
	{}

	template <class T>
	Delegate(T* obj, typename MethodStub<T>::FuncType func)
		: objPtr(obj)
		, funcPtr(*(void**)&func)
		, stubPtr(&MethodStub<T>::invoke)
	{}

	Delegate(typename FunctionStub::FuncType func)
		: objPtr(nullptr)
		, funcPtr(*(void**)&func)
		, stubPtr(&FunctionStub::invoke)
	{}

	Delegate(ScriptFunction* func)
		: objPtr(nullptr)
		, funcPtr(func)
		, stubPtr(&ScriptStub::invoke)
	{}

	Delegate(const Delegate& cpy) = default;

	~Delegate()
	{
		unbind();
	}

	template <class T>
	void bind(T* obj, typename MethodStub<T>::FuncType func)
	{
		objPtr = obj;
		funcPtr = *(void**)&func;
		stubPtr = &MethodStub<T>::invoke;
	}

	void bind(typename FunctionStub::FuncType func)
	{
		objPtr = nullptr;
		funcPtr = *(void**)&func;
		stubPtr = &FunctionStub::invoke;
	}

	void unbind()
	{
		objPtr = nullptr;
		funcPtr = nullptr;
		stubPtr = nullptr;
	}

	inline bool isBound() const
	{
		return (stubPtr != 0 && funcPtr != 0);
	}

	bool operator()(Args... params)
	{
		if (isBound())
		{
			(*stubPtr)(objPtr, funcPtr, params...);
			return true;
		}
		return false;
	}

	bool compare(const void* obj, const void* func) const
	{
		return (objPtr == obj) && (funcPtr == func);
	}

	bool compareObject(const void* obj) const
	{
		return (objPtr == obj);
	}

	stub_type stubPtr = 0;
	void* objPtr = 0;
	void* funcPtr = 0;
};


template <class... Args>
struct MulticastDelegate
{
	typedef Delegate<Args...> DelegateType;
	typedef std::vector<DelegateType> DelegateVec;

	~MulticastDelegate()
	{
		clear();
	}

	void clear()
	{
		mInvokeVec.clear();
	}

	template<class T>
	MulticastDelegate& add(T* obj, typename Delegate<Args...>::MethodStub<T>::FuncType func)
	{
		const void* funcPtr = *(const void**)&func;
		for (size_t i = 0; i < mInvokeVec.size(); ++i)
		{
			if (mInvokeVec[i].compare(obj, funcPtr))
			{
				return *this;
			}
		}

		mInvokeVec.emplace_back(obj, func);

		return *this;
	}

	MulticastDelegate& add(ScriptFunction* func)
	{
		if (func)
			mInvokeVec.emplace_back(func);

		return *this;
	}

	template<class T>
	MulticastDelegate& remove(T* obj, typename Delegate<Args...>::MethodStub<T>::FuncType func)
	{
		const void* funcPtr = *(const void**)&func;

		for (size_t i = 0; i < mInvokeVec.size();)
		{
			if (mInvokeVec[i].compare(obj, funcPtr))
			{
				mInvokeVec[i].unbind();
				mInvokeVec[i] = mInvokeVec.back();
				mInvokeVec.pop_back();
			}
			else
				++i;
		}

		return *this;
	}

	template<class T>
	MulticastDelegate& remove(T* obj)
	{
		for (size_t i = 0; i < mInvokeVec.size();)
		{
			if (mInvokeVec[i].compareObject(obj))
			{
				mInvokeVec[i].unbind();
				mInvokeVec[i] = mInvokeVec.back();
				mInvokeVec.pop_back();
			}
			else
				++i;
		}

		return *this;
	}

	void operator () (Args... params)
	{
		bool listDirty = false;

		for (int i = (int)mInvokeVec.size() - 1; i >= 0; --i)
		{
			if (!mInvokeVec[i](params...))
			{
				listDirty = true;
			}
		}

		if (listDirty)
		{
			for (int i = (int)mInvokeVec.size() - 1; i >= 0; --i)
			{
				if (!mInvokeVec[i].isBound())
				{
					mInvokeVec[i] = mInvokeVec.back();
					mInvokeVec.pop_back();
				}
			}
		}
	}

	DelegateVec mInvokeVec;
};

#endif