/* SPDX-License-Identifier: LGPL-2.0-or-later OR MIT */
/*
 * Macros to generate the definition of ffs- and fls- related functions,
 * with macro parameetrs to customize their behaviors.
 *
 * Authors:
 *     Yueh-Shun Li <shamrocklee@posteo.net>
 */

#ifndef _ASM_GENERIC_BITOPS_GEN_FFLS_BRANCHLESS_H_
#define _ASM_GENERIC_BITOPS_GEN_FFLS_BRANCHLESS_H_

#include <linux/compiler.h>

/* Preserve the most significant bit set of X and unset others.
 * Return zero when non set.
 */
#define __preserve_msb_branchless(X, type)                \
	___preserve_msb_branchless(X, type, sizeof(type), \
				   __UNIQUE_ID(_is_nonzero))
#define ___preserve_msb_branchless(X, type, x_size, _is_nonzero) \
	do {                                                     \
		type _is_nonzero = !!(X);                        \
		(X) >>= 1;                                       \
		(X) |= (X) >> 1;                                 \
		(X) |= (X) >> 2;                                 \
		(X) |= (X) >> 4;                                 \
		(X) |= (X) >> (((x_size) > 1) << 3);             \
		(X) |= (X) >> (((x_size) > 2) << 4);             \
		(X) |= (X) >> (((x_size) > 4) << 5);             \
		(X) += _is_nonzero;                              \
	} while (0)

/* Preserve the least significant bit set of X and unset others.
 * Return zero when non set.
 */
#define __preserve_lsb_branchless(X, type)                \
	___preserve_lsb_branchless(X, type, sizeof(type), \
				   __UNIQUE_ID(_x_upperfilled))
#define ___preserve_lsb_branchless(X, type, x_size, _x_upperfilled) \
	do {                                                        \
		(X) |= (X) << 1;                                    \
		(X) |= (X) << 2;                                    \
		(X) |= (X) << 4;                                    \
		(X) |= (X) << (((x_size) > 1) << 3);                \
		(X) |= (X) << (((x_size) > 2) << 4);                \
		(X) |= (X) << (((x_size) > 4) << 5);                \
		type _x_upperfilled = (X);                          \
		(X) = ~(X) + (!!_x_upperfilled);                    \
		(X) &= _x_upperfilled;                              \
	} while (0)

/* Take in an unsigned integer with at most one bit set, and then
 * output the 1-based digit of that bit. Return 0 if no bits are set.
 *
 * Bits higher than (2**64 - 1) will be truncated.
 *
 * Determine the result bit value through binary search with the following
 * masks:
 *
 * 0b01010101'01010101'01010101'01010101'01010101'01010101'01010101'01010101u
 * 0b01100110'01100110'01100110'01100110'01100110'01100110'01100110'01100110u
 * 0b01111000'01111000'01111000'01111000'01111000'01111000'01111000'01111000u
 * 0b01111111'10000000'01111111'10000000'01111111'10000000'01111111'10000000u
 * 0b01111111'11111111'10000000'00000000'01111111'11111111'10000000'00000000u
 * 0b01111111'11111111'11111111'11111111'10000000'00000000'00000000'00000000u
 * 0b10000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000u
 *
 * This macro currently supports up to 64-bit integer input.
 * Add extra masks and lengthen previous ones to support wider input type
 * (e.g. 128 bits).
 */
#define __locate_bit_branchless(x, Tin, Tout) \
	___locate_bit_branchless(x, Tin, Tout, (Tin) ~(Tin)0)
#define ___locate_bit_branchless(x, Tin, Tout, in_111)                     \
	((Tout)(((!!((x) & ((Tin)((in_111)&0x5555555555555555u)))) << 0) + \
		((!!((x) & ((Tin)((in_111)&0x6666666666666666u)))) << 1) + \
		((!!((x) & ((Tin)((in_111)&0x7878787878787878u)))) << 2) + \
		((!!((x) & ((Tin)((in_111)&0x7f807f807f807f80u)))) << 3) + \
		((!!((x) & ((Tin)((in_111)&0x7fff80007fff8000u)))) << 4) + \
		((!!((x) & ((Tin)((in_111)&0x7fffffff80000000u)))) << 5) + \
		((!!((x) & ((Tin)((in_111)&0x8000000000000000u)))) << 6)))

/**
 * rounddown_pow_of_two_inplace_branchless() - Branchlessly round an integer
 * 	variable down to nearest power of two.
 * @x: The variable to round down.
 */
#define rounddown_pow_of_two_inplace_branchless(x) \
	__preserve_msb_branchless(x, typeof(x))

/**
 * gen_fls_branchless() - Define a branchless fls function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @start_from: The value of the index representing the 2^0 bit,
 *              i.e. the return value of the defining function taking 1.
 *              The value must be either 0 or 1.
 *              start_from = 1 is slightly more efficient and more informative.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the index of the last bit (most significant bit) set.
 *
 * When the defining function takes 0, it returns 0.
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_fls_branchless(fn_name, in_type, out_type, start_from)     \
	__attribute_const__ out_type fn_name(in_type x)                \
	{                                                              \
		__preserve_msb_branchless(x, in_type);                 \
		return __locate_bit_branchless(x, in_type, out_type) - \
		       ((!(start_from)) & (!x));                       \
	}

/**
 * gen_ffs_branchless() - Define a branchless ffs function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @start_from: The value of the index representing the 2^0 bit,
 *              i.e. the return value of the defining function taking 1.
 *              The value must be either 0 or 1.
 *              start_from = 1 is slightly more efficient and more informative.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the index of the first bit (least significant bit) set.
 *
 * When the defining function takes 0, it returns 0.
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_ffs_branchless(fn_name, in_type, out_type, start_from)     \
	__attribute_const__ out_type fn_name(in_type x)                \
	{                                                              \
		__preserve_lsb_branchless(x, in_type);                 \
		return __locate_bit_branchless(x, in_type, out_type) - \
		       ((!(start_from)) && (!x));                      \
	}

/**
 * gen_flz_branchless() - Define a branchless flz function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @start_from: The value of the index representing the 2^0 bit,
 *              i.e. the return value of the defining function taking 1.
 *              The value must be either 1 or 0.
 *              start_from = 1 is slightly more efficient and more informative.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the index of the last (most significant) bit unset.
 *
 * When the defining function takes 0, it returns 0.
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_flz_branchless(fn_name, in_type, out_type, start_from)   \
	__gen_flz_branchless(fn_name, in_type, out_type, start_from, \
			     __UNIQUE_ID(_temp_fls))
#define __gen_flz_branchless(fn_name, in_type, out_type, start_from,     \
			     _temp_fls)                                  \
	static __always_inline gen_fls_branchless(_temp_fls, in_type,    \
						  out_type, start_from); \
	__attribute_const__ out_type fn_name(in_type x)                  \
	{                                                                \
		return _temp_fls(~x);                                    \
	}

/**
 * gen_ffz_branchless() - Define a branchless ffz function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @start_from: The value of the index representing the 2^0 bit,
 *              i.e. the return value of the defining function taking 1.
 *              The value must be either 1 or 0.
 *              start_from = 1 is slightly more efficient and more informative.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the index of the first (least significant) bit unset.
 *
 * When the defining function takes 0, it returns 0.
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_ffz_branchless(fn_name, in_type, out_type, start_from)   \
	__gen_ffz_branchless(fn_name, in_type, out_type, start_from, \
			     __UNIQUE_ID(_temp_ffs))
#define __gen_ffz_branchless(fn_name, in_type, out_type, start_from,     \
			     _temp_ffs)                                  \
	static __always_inline gen_ffs_branchless(_temp_ffs, in_type,    \
						  out_type, start_from); \
	__attribute_const__ out_type fn_name(in_type x)                  \
	{                                                                \
		return _temp_ffs(~x);                                    \
	}

/**
 * gen_clz_branchless() - Define a branchless clz function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @nonzero_exception: Toggle the exceptional return when the defining function takes 0.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the number of leading zero bits (unset bits preceding the
 * most significant bit).
 *
 * When the defining function takes 0, it returns
 * * 0 if nonzero_exception == 0,
 * * (size(in_type) * 8) otherwise (preferred).
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_clz_branchless(fn_name, in_type, out_type, nonzero_exception)   \
	__gen_clz_branchless(fn_name, in_type, out_type, nonzero_exception, \
			     __UNIQUE_ID(_temp_fls))
#define __gen_clz_branchless(fn_name, in_type, out_type, nonzero_exception, \
			     _temp_fls)                                     \
	static __always_inline gen_fls_branchless(_temp_fls, in_type,       \
						  out_type, 1);             \
	__attribute_const__ out_type fn_name(in_type x)                     \
	{                                                                   \
		return (sizeof(in_type) * ((!!(nonzero_exception)) * 8)) -  \
		       (_temp_fls(x));                                      \
	}

/**
 * gen_ctz_branchless() - Define a branchless ctz function.
 * @fn_name: The name of the defining function.
 * @in_type: The input type of the defining function. It must be an integer type.
 * @out_type: The output type of the defining function. It must be an integer type.
 * @nonzero_exception: Toggle the exceptional return when the defining function takes 0.
 *
 * The defining function takes an integer as its input arguments, and
 * returns the number of trailing zero bits (unset bits following the
 * least significant bit).
 *
 * When the defining function takes 0, it returns
 * * 0 if nonzero_exception == 0,
 * * (size(in_type) * 8) otherwise (preferred).
 *
 * Additional type qualifiers (e.g. static) and attributes (e.g. __always_inline)
 * can be prepended before the call site of this macro.
 */
#define gen_ctz_branchless(fn_name, in_type, out_type, nonzero_exception)   \
	__gen_ctz_branchless(fn_name, in_type, out_type, nonzero_exception, \
			     __UNIQUE_ID(_temp_ffs))
#define __gen_ctz_branchless(fn_name, in_type, out_type, nonzero_exception, \
			     _temp_ffs)                                     \
	static __always_inline gen_ffs_branchless(_temp_ffs, in_type,       \
						  out_type, 1);             \
	__attribute_const__ out_type fn_name(in_type x)                     \
	{                                                                   \
		return (_temp_ffs(x)) - ((!nonzero_exception) && (x));      \
	}

#endif /* _ASM_GENERIC_BITOPS_GEN_FFLS_BRANCHLESS_H_ */
