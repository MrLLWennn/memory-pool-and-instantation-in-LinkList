#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H


#include<type_traits>


namespace my_memory_pool
{

	template<class T, T v>
		struct m_integral_construct
		{
			static constexpr T value = v;
		};

	template<bool b>
		using m_bool_constant = m_integral_construct<bool, b>;

	typedef m_bool_constant<true> m_true_type;
	typedef m_bool_constant<false> m_false_type;

};//namespace my_memory_pool


#endif // TYPE_TRAITS_H
