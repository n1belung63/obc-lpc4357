#pragma once

template <typename T>
class Singleton {
public:
	static T & GetInstance() {
		static T instance;
		return instance;
	}
	Singleton(const Singleton<T> &) = delete;
	const Singleton<T> & operator=(const Singleton<T> &) = delete;
protected:
	Singleton() = default;
};