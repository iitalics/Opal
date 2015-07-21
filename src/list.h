#pragma once
#include <initializer_list>
#include <iterator>
#include <vector>

template <typename T>
class list
{
public:
	struct pair
	{
		T head;
		std::shared_ptr<pair> tail;

		pair (T _head, const std::shared_ptr<pair>& _tail = nullptr)
			: head(_head), tail(_tail) {}
	};
	using pairPtr = std::shared_ptr<pair>;
	struct iterator
	{
		T operator* () const { return _pair->head; }
		bool operator!= (const iterator& other) const { return _pair != other._pair; }
		iterator& operator++ () { _pair = _pair->tail; return *this; }

	private:
		friend class list;
		iterator (pairPtr p)
			: _pair(p) {}
		pairPtr _pair;
	};

	inline list ()
		: _pair(nullptr) {}
	inline explicit list (T hd, const list& tail = list())
		: _pair(std::make_shared<pair>(hd, tail._pair)) {}
	inline list (const list& other)
		: _pair(other._pair) {}
	list (const std::initializer_list<T>& init)
		: _pair(nullptr)
	{
		for (auto it = init.end(); it-- != init.begin(); )
			_pair = std::make_shared<pair>(*it, _pair);
	}
	list (const std::vector<T>& init)
		: _pair(nullptr)
	{
		for (auto it = init.end(); it-- != init.begin(); )
			_pair = std::make_shared<pair>(*it, _pair);
	}

	inline ~list () {}

	inline bool nil () const { return _pair == nullptr; }
	T head () const
	{
		if (nil())
			throw std::runtime_error("nil has no head");
		else
			return _pair->head;
	}
	list tail () const
	{
		if (nil())
			throw std::runtime_error("nil has no tail");
		else
			return list(_pair->tail);
	}

	iterator begin () const { return iterator(_pair); }
	iterator end () const { return iterator(nullptr); }


	size_t length () const
	{
		size_t n = 0;
		for (auto p = _pair; p != nullptr; p = p->tail)
			n++;
		return n;
	}
	T ref (size_t i) const
	{
		if (nil())
			throw std::runtime_error("index out of bounds");
		else if (i == 0)
			return head();
		else // please tail call this
			return tail().ref(i - 1);
	}
	list<T> reverse () const
	{
		auto p = pairPtr(nullptr);
		for (auto t : *this)
			p = std::make_shared<pair>(t, p);
		return list(p);
	}
	template <typename U = list<T>, typename F>
	U fold (F fn, U z) const
	{
		for (auto x : *this)
			z = fn(z, x);
		return z;
	}
	template <typename U = T, typename TF>
	list<U> map (TF transform) const
	{
		std::vector<U> res;
		res.reserve(size());
		for (auto x : *this)
			res.push_back(transform(x));
		return list<U>(res);
	}
	template <typename PF>
	list<T> filter (PF pred) const
	{
		std::vector<T> res;
		for (auto x : *this)
			if (pred(x))
				res.push_back(x);
		return list<T>(res);
	}
	template <typename PF>
	bool all (PF pred) const
	{
		for (auto x : *this)
			if (!pred(x))
				return false;
		return true;
	}
	template <typename PF>
	bool some (PF pred) const
	{
		for (auto x : *this)
			if (pred(x))
				return true;
		return false;
	}

	// std::vector-feeling method aliases
	inline bool empty () const { return nil(); }
	inline size_t size () const { return length(); }
	inline T front () const { return head(); }
	inline T operator[] (size_t i) const { return ref(i); }
	inline list& operator++ () { _pair = tail()._pair; return *this; }
private:
	explicit list (const pairPtr& p)
		: _pair(p) {}

	pairPtr _pair;
};

template <typename T>
std::ostream& operator<< (std::ostream& s, const list<T>& lst)
{
	bool first = true;
	s << "[";
	for (auto t : lst)
	{
		if (first)
		{
			s << " ";
			first = false;
		}
		else
			s << ", ";

		s << t;
	}
	return s << " ]";
}