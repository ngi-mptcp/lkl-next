/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __ASM_GENERIC_EXPORT_H
#define __ASM_GENERIC_EXPORT_H

#ifndef KSYM_FUNC
#define KSYM_FUNC(x) x
#endif
#ifdef CONFIG_HAVE_ARCH_PREL32_RELOCATIONS
#define KSYM_ALIGN 4
#elif defined(CONFIG_64BIT)
#define KSYM_ALIGN 8
#else
#define KSYM_ALIGN 4
#endif
#ifndef KCRC_ALIGN
#define KCRC_ALIGN 4
#endif

#ifdef CONFIG_HAVE_UNDERSCORE_SYMBOL_PREFIX
#define KSYM(name) _##name
#else
#define KSYM(name) name
#endif

.macro __put, val, name
#ifdef CONFIG_HAVE_ARCH_PREL32_RELOCATIONS
	.long	\val - ., \name - ., 0
#elif defined(CONFIG_64BIT)
	.quad	\val, \name, 0
#else
	.long	\val, \name, 0
#endif
.endm

/*
 * note on .section use: we specify progbits since usage of the "M" (SHF_MERGE)
 * section flag requires it. Use '%progbits' instead of '@progbits' since the
 * former apparently works on all arches according to the binutils source.
 */

.macro ___EXPORT_SYMBOL name,val,sec
#ifdef CONFIG_MODULES
	.section ___ksymtab\sec+\name,"a"
	.balign KSYM_ALIGN
KSYM(__ksymtab_\name):
	__put \val, KSYM(__kstrtab_\name)
	.previous
	.section __ksymtab_strings,"aMS",%progbits,1
KSYM(__kstrtab_\name):
#ifdef CONFIG_HAVE_UNDERSCORE_SYMBOL_PREFIX
	.asciz "_\name"
#else
	.asciz "\name"
#endif
	.previous
#ifdef CONFIG_MODVERSIONS
	.section ___kcrctab\sec+\name,"a"
	.balign KCRC_ALIGN
#if defined(CONFIG_MODULE_REL_CRCS)
	.long KSYM(__crc_\name) - .
#else
	.long KSYM(__crc_\name)
#endif
	.weak KSYM(__crc_\name)
	.previous
#endif
#endif
.endm

#if defined(CONFIG_TRIM_UNUSED_KSYMS)

#include <linux/kconfig.h>
#include <generated/autoksyms.h>

.macro __ksym_marker sym
	.section ".discard.ksym","a"
__ksym_marker_\sym:
	 .previous
.endm

#define __EXPORT_SYMBOL(sym, val, sec)				\
	__ksym_marker sym;					\
	__cond_export_sym(sym, val, sec, __is_defined(__KSYM_##sym))
#define __cond_export_sym(sym, val, sec, conf)			\
	___cond_export_sym(sym, val, sec, conf)
#define ___cond_export_sym(sym, val, sec, enabled)		\
	__cond_export_sym_##enabled(sym, val, sec)
#define __cond_export_sym_1(sym, val, sec) ___EXPORT_SYMBOL sym, val, sec
#define __cond_export_sym_0(sym, val, sec) /* nothing */

#else
#define __EXPORT_SYMBOL(sym, val, sec) ___EXPORT_SYMBOL sym, val, sec
#endif

#define EXPORT_SYMBOL(name)					\
	__EXPORT_SYMBOL(name, KSYM_FUNC(KSYM(name)),)
#define EXPORT_SYMBOL_GPL(name) 				\
	__EXPORT_SYMBOL(name, KSYM_FUNC(KSYM(name)), _gpl)
#define EXPORT_DATA_SYMBOL(name)				\
	__EXPORT_SYMBOL(name, KSYM(name),)
#define EXPORT_DATA_SYMBOL_GPL(name)				\
	__EXPORT_SYMBOL(name, KSYM(name),_gpl)

#endif
