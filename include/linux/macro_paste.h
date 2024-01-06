/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 * Macros for indirect token pasting (concatenation).
 *
 * Authors:
 * 	Yueh-Shun Li <shamrocklee@posteo.net>
 */

#ifndef _LINUX_MACRO_PASTE_H
#define _LINUX_MACRO_PASTE_H

/**
 * PASTE() - Indirect token pasting.
 * @A: The left-hand-side token to concatenate.
 * @B: The right-hand-side token to concatenate.
 *
 * Expand two tokens and paste/concatenate into one. The joint token will also
 * be expanded.
 *
 * Doing two levels allows each parameters to be a macro itself.
 *
 * For example, compile with -DFOO=bar, PASTE(FOO, BAR) converts to bazBAR;
 * compile with -DFOO=bar -DbazBAR=42, PASTE(FOO, BAR) converts to 42.
 *
 * Return:
 * 	The the expansion of the token joint from the expansions of
 * 	both inputs.
 * The name "PASTE" is chosen over "CONCATENATE" or "CONCAT", since it's the
 * shortest among the three.
 */
#define PASTE(A, B) __PASTE_DIRECTLY(A, B)
#define __PASTE_DIRECTLY(A, B) A##B

#endif /* _LINUX_MACRO_PASTE_H */
