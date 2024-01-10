#pragma once

#include <type_traits>
#include <iostream>

namespace filter
{
	template<typename T, T __Min, T __Max, bool __Make_Odd = false>
	class filter_parameter_integral {
		static_assert(std::is_integral<T>::value, "Type must be integral");

	public:
		filter_parameter_integral() : _value(__Min) { }

		void operator=(const T& val)
		{
			_value = val;

			if constexpr (__Make_Odd)
				_value |= 1;

			_value = _value < __Min ? __Min : _value;
			_value = _value > __Max ? __Max : _value;
		}

		const T value() const
		{
			return _value;
		}

		//T filter(T value) {
		//    if (value < min) return min;
		//    if (value > max) return max;

		//    if constexpr (make_odd && std::is_same<T, int>::value && std::is_same<decltype(min), decltype(max)>::value) {
		//        if (value % 2 == 0) {
		//            return (value + 1 <= max) ? (value + 1) : value;
		//        }
		//    }

		//    return value;
		//}

	private:
		T _value;
	};

	template<typename T, T __Min, T __Max>
	class filter_parameter_non_integral {
		static_assert(!std::is_integral<T>::value, "Type must not be integral");

	public:
		filter_parameter_non_integral() : _value(__Min) { }

		void operator=(const T& val)
		{
			static_assert(std::is_same_v<T, std::decay_t<decltype(val)>>, "assigned value must match template type");

			_value = val;
			_value = _value < __Min ? __Min : _value;
			_value = _value > __Max ? __Max : _value;
		}

		const T value() const
		{
			return _value;
		}

	private:
		T _value;
	};

	template<typename T, T min, T max, bool make_odd = false>
	using filter_parameter = std::conditional_t<std::is_integral<T>::value,
		filter_parameter_integral<T, min, max, make_odd>,
		filter_parameter_non_integral<T, min, max>>;
}