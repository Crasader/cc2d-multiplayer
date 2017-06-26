#ifndef __MULTICASTDELEGATE_H__
#define __MULTICASTDELEGATE_H__

/*template <class T>
class MulticastDelegate
{
public:
	struct EventFunc
	{
		EventFunc(const T& _func, void* objPtr)
			: func(_func)
			, obj(objPtr)
		{}

		T func;
		void* obj;
	};

	void add(const T& func, void* objPtr)
	{
		funcs.emplace_back(func, objPtr);
	}

	void remove(void* objPtr)
	{
		auto it = funcs.begin();
		while (it != funcs.end())
		{
			if ((*it).obj == objPtr)
			{
				it = funcs.erase(it);
			}
			else
				++it;
		}
	}

	std::vector<EventFunc> funcs;
};*/

#endif