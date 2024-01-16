#pragma once

#include <type_traits>
#include <iostream>

namespace filter
{
	template<typename T, const T __Min, const T __Max, const bool __Make_Odd = false>
	class filter_parameter_integral {
		static_assert(std::is_integral<T>::value, "Type must be integral");

	public:
		filter_parameter_integral() : _value(__Min) { }
		filter_parameter_integral(const T val) : _value(val) { _value = value(); }

		void operator=(const T& val)
		{
			static_assert(std::is_same_v<T, std::decay_t<decltype(val)>>, "assigned value must match template type");
			
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

		constexpr T min() const
		{
			return __Min;
		}

		constexpr T max() const
		{
			return __Max;
		}

	private:
		T _value;
	};

	template<typename T, const T __Min, const T __Max>
	class filter_parameter_non_integral {
		static_assert(!std::is_integral<T>::value, "Type must not be integral");

	public:
		filter_parameter_non_integral() : _value(__Min) { }
		filter_parameter_non_integral(const T val) : _value(val) { _value = value(); }

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

		constexpr T min() const
		{
			return __Min;
		}

		constexpr T max() const
		{
			return __Max;
		}

	private:
		T _value;
	};

	template<typename T, const T min, const T max, const bool make_odd = false>
	using filter_parameter = std::conditional_t<std::is_integral<T>::value,
		filter_parameter_integral<T, min, max, make_odd>,
		filter_parameter_non_integral<T, min, max>>;
}