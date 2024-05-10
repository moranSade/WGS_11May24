/*******************************************************************************
  ed_lib.h  Developed by Dafocus - http://www.dafocus.com/
*******************************************************************************/
#ifndef __ED_LIB_H
#define __ED_LIB_H
#include "ed_data.h"
#ifdef __cplusplus
	extern "C" {
#endif

/* Define ED_REVERSED_INTEGER to have integer coded placing the least significant
   bit on the right */
/* #define ED_REVERSED_INTEGER */
#define ED_MISSING_OPTIONAL             (32767)
#define ED_NO_ERRORS                         0
#define ED_SYNTAX_ERROR                    (-1)
#define ED_UNKNOWN_MESSAGE                 (-2)
#define ED_UNKNOWN_IEI                     (-3)
#define ED_MESSAGE_TOO_SHORT               (-4)
#define ED_MISSING_REQUIRED_CONDITIONAL_IE (-5)
#define ED_FOUND_UNEXPECTED_CONDITIONAL_IE (-6)
#define ED_MANDATORY_IE_SYNTAX_ERROR       (-7)
#define ED_CONDITIONAL_IE_SYNTAX_ERROR     (-8)
#define ED_OPTIONAL_IE_SYNTAX_ERROR        (-9)
#define ED_FIELD_OUT_OF_RANGE             (-10)
#define ED_PACKEDBUFFER_ERROR             (-11)
#define ED_IE_SIZE_ERROR                  (-12)
#define ED_ARRAY_OVERFLOW                 (-13)
#define ED_OUT_OF_MEMORY                  (-14)
#define ED_CONST const

/*------------------------------------------------------------------------------
	Function call. Use this to add export/call convetion (e.g. "pascal") 
	modifiers.
------------------------------------------------------------------------------*/
#ifndef ED_EXPORT_CALL
#define ED_EXPORT_CALL(ret,name) ret name
#endif

/*------------------------------------------------------------------------------
	Function used to set a memory zone to 0
------------------------------------------------------------------------------*/
#ifndef ED_RESET_MEM
#include <string.h>
#define ED_STRING_H_INCLUDED
#define ED_RESET_MEM(ptr,len) memset (ptr, 0, len)
#endif

/*------------------------------------------------------------------------------
	Function used to copy memory
------------------------------------------------------------------------------*/
#ifndef ED_MEM_COPY
#ifndef ED_STRING_H_INCLUDED
#include <string.h>
#define ED_STRING_H_INCLUDED
#endif
#define ED_MEM_COPY(dest,src,len) memcpy (dest, src, len)
#endif

/*------------------------------------------------------------------------------
	Function used to move memory. This must support overlapping memory
	areas, as in ANSI memmove function.
------------------------------------------------------------------------------*/
#ifndef ED_MEM_MOVE
#ifndef ED_STRING_H_INCLUDED
#include <string.h>
#define ED_STRING_H_INCLUDED
#endif
#define ED_MEM_MOVE(dest,src,len) memmove (dest, src, len)
#endif

/*------------------------------------------------------------------------------
	Define this as empty to have a slightly more optimized code which could
	show a "unused variable" warning.
	
	Do a "#define ED_WARNING_REMOVER(text)" in ed_user.h to disable it.
------------------------------------------------------------------------------*/
#ifndef ED_WARNING_REMOVER
#define ED_WARNING_REMOVER(text) ((void)(text))
#endif

#ifndef ED_EXTRAPARAMS_DECL
#define ED_EXTRAPARAMS_DECL
#endif

#ifndef ED_EXTRAPARAMS_CALL
#define ED_EXTRAPARAMS_CALL
#endif

#ifndef ED_EXTRAPARAMS_CODE
#define ED_EXTRAPARAMS_CODE
#endif

/*------------------------------------------------------------------------------
	This function copies given number of bits starting from given SourceOffset 
	from "Source" buffer to "Destination" buffer.  Bits are aligned on destination
	buffer starting from bit "DestOffset".
	
	Bit-positions are expressed as follows:
	 - bit 0 is the highest (most significant) bit of octet 0;
	 - bit 7 is the lowest (less significant) bit of octet 0;
	 - bit 8 is the highest (most significant) bit of octet 1;
	...and so on.
	
	Both Lenght and Offsets are expressed in bits.
------------------------------------------------------------------------------*/
void EDCopyBitsInternal (void* Destination, ED_EXLONG DestOffset, const void* Source, ED_EXLONG SourceOffset, ED_EXLONG Length);
#define EDCopyBits(Destination, DestOffset, Source, SourceOffset, Length) EDCopyBitsInternal (Destination, (ED_EXLONG)(DestOffset), Source, (ED_EXLONG)(SourceOffset), (ED_EXLONG)(Length))

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' integer to
	'Destination' ad offset 'Offset'.
	Bits are saved placing the most significant one on the left, unless
	ED_REVERSED_INTEGER is defined.
------------------------------------------------------------------------------*/
void EDIntToBitsInternal (void* Destination, ED_EXLONG Offset, int Source, int Count);
#define EDIntToBits(Destination, Offset, Source, Count) EDIntToBitsInternal (Destination, (ED_EXLONG)(Offset), (int)(Source), (int)(Count))

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' char buffer
	from offset 'Offset'. Read integer is returned.
	Bits are considered with the most significant one on the left, unless
	ED_REVERSED_INTEGER is defined.
------------------------------------------------------------------------------*/
int EDBitsToIntInternal (const void* Source, ED_EXLONG Offset, int Count);
#define EDBitsToInt(Source, Offset, Count) EDBitsToIntInternal (Source, (ED_EXLONG)(Offset), (int)(Count))

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' integer to
	'Destination' ad offset 'Offset'.
	Bits are saved placing the least significant one on the left, unless
	ED_REVERSED_INTEGER is defined.
------------------------------------------------------------------------------*/
void EDIntToBitsReversedInternal (void* Destination, ED_EXLONG Offset, int Source, int Count);
#define EDIntToBitsReversed(Destination, Offset, Source, Count) EDIntToBitsReversedInternal (Destination, (ED_EXLONG)(Offset), (int)(Source), (int)(Count))

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' char buffer
	from offset 'Offset'. Read integer is returned.
	Bits are considered with the least significant one on the left, unless
	ED_REVERSED_INTEGER is defined.
------------------------------------------------------------------------------*/
int EDBitsToIntReversedInternal (const void* Source, ED_EXLONG Offset, int Count);
#define EDBitsToIntReversed(Source, Offset, Count) EDBitsToIntReversedInternal (Source, (ED_EXLONG)(Offset), (int)(Count))

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' char buffer
	from offset 'Offset'. Read integer is returned.
	Implemented according to IEEE Std 802.11-2012 chapter "8.2.2 Conventions".
------------------------------------------------------------------------------*/
extern int EDBitsToInt_IEEE (const void* Source, ED_EXLONG SourceOffset, int Count);

/*------------------------------------------------------------------------------
	This function copies 'Count' less significant bits of 'Source' integer to
	'Destination' ad offset 'Offset'.
	Implemented according to IEEE Std 802.11-2012 chapter "8.2.2 Conventions".
------------------------------------------------------------------------------*/
extern void EDIntToBits_IEEE (void* Destination, ED_EXLONG DestOffset, int Source, int Count);

/*------------------------------------------------------------------------------
	This function copies given number of bits starting from given SourceOffset 
	from "Source" buffer to "Destination" buffer.  Bits are aligned on destination
	buffer starting from bit "DestOffset".
	
	Bit-positions are expressed in IEEE format Std 802.11-2012 chapter 
	"8.2.2 Conventions" as follows:
	 - bit 0 is the lowest (least significant) bit of octet 0;
	 - bit 7 is the highest (most significant) bit of octet 0;
	 - bit 8 is the lowest (least significant) bit of octet 1;
	...and so on.
	
	Both Lenght and Offsets are expressed in bits.
------------------------------------------------------------------------------*/
void EDCopyBits_IEEE (void* Destination, ED_EXLONG DestOffset, const void* Source, ED_EXLONG SourceOffset, ED_EXLONG Length);

/*------------------------------------------------------------------------------
	Encodes a "definite form" length value. See X.690 8.1.3.3.
	It returns the number of bits used to encode the value.
	"Offset" is expressed in bits.
------------------------------------------------------------------------------*/
int EDEncodeDefiniteFormLength (void* Destination, ED_EXLONG Offset, unsigned ED_EXLONG Value);

/*------------------------------------------------------------------------------
	Decodes a "definite form" length value. See X.690 8.1.3.3.
	It returns the number of bits used to decode the value or ED_IE_SIZE_ERROR
	in case of error.
	"Offset" is expressed in bits.
	It returns the value in "Value". 
------------------------------------------------------------------------------*/
int EDDecodeDefiniteFormLength (const void* Source, ED_EXLONG Offset, ED_EXLONG* Value);

/*------------------------------------------------------------------------------
	These macros can be used in custom e/d functions.
------------------------------------------------------------------------------*/
#define CURPOS CurrOfs
#define SHIFT_INT(bits) (CURPOS+=(bits), EDBitsToInt (Buffer, CURPOS-(bits), (bits)))
#define SHIFT_INT_REV(bits) (CURPOS+=(bits), EDBitsToIntReversed (Buffer, CURPOS-(bits), (bits)))
#define SHIFT_INT_IEEE(bits) (CURPOS+=(bits), EDBitsToInt_IEEE (Buffer, CURPOS-(bits), (bits)))
#define SHIFT_BIN(dest,bits) {EDCopyBits ((dest), 0, Buffer, CURPOS, (bits)); CURPOS+=(bits);}
#define SHIFT_BIN_IEEE(dest,bits) {EDCopyBits_IEEE ((dest), 0, Buffer, CURPOS, (bits)); CURPOS+=(bits);}
#define PUSH_INT(val, bits)	(EDIntToBits (Buffer, CURPOS, val, (bits)), CURPOS+=(bits))
#define PUSH_BOOL(val)	{EDIntToBits (Buffer, CURPOS, ((val)==ED_FALSE? 0:1), 1); CURPOS++;}
#define PUSH_INT_IEEE(val, bits)	(EDIntToBits_IEEE (Buffer, CURPOS, val, (bits)), CURPOS+=(bits))
#define PUSH_BOOL_IEEE(val)	{EDIntToBits_IEEE (Buffer, CURPOS, ((val)==ED_FALSE? 0:1), 1); CURPOS++;}
#define PUSH_INT_REV(val, bits)	{EDIntToBitsReversed (Buffer, CURPOS, (val), (bits)); CURPOS+=bits;}
#define PUSH_BIN(val, bits)	{EDCopyBits (Buffer, CURPOS, (val), 0, (bits)); CURPOS+=(bits);}
#define PUSH_BIN_IEEE(val, bits) {EDCopyBits_IEEE (Buffer, CURPOS, (val), 0, (bits)); CURPOS+=(bits);}
#define PRESENT(a) a##_Present
#define BITS_TO_OCTET_PAD(n) ((8-((n) & 7))&7)

/*------------------------------------------------------------------------------
	This macro is invoked when a error has to be reported.
------------------------------------------------------------------------------*/
#ifndef ED_SIGNAL_ERROR
#include <stdio.h>
#include "logManager.h"
#define ED_SIGNAL_ERROR(errtext) logPrint (LOG_DEBUG, errtext);
#endif

/*------------------------------------------------------------------------------
	This structure is used to store known IE data. This is used to skip
	unexpected informatio elements.
------------------------------------------------------------------------------*/
typedef struct SEDKnownIE {
	/* Value of the IEI */
	unsigned char IEI;

	/* Mask - When comparing an unknown IEI "i", we do: (i & IEIMask) == IEI */
	unsigned char IEIMask;
	
	/* Length is >=0 if the length of the IE is fixed and known. In this
	   case, Length is its length in bits. If Length<0, the field is TLV
	   and Length is the negative length of the "L" field in bits. */
	int Length;
} TEDKnownIE;


/*------------------------------------------------------------------------------
	TLV Skip list item type.
------------------------------------------------------------------------------*/
typedef struct SEDTLVSkipItem {
	/* Value of the IEI */
	unsigned char IEI;

	/* Mask - When comparing an unknown IEI "i", we do: (i & IEIMask) == IEI */
	unsigned char IEIMask;
	
	/* Type of the field (see ED_SKIPLIST_xxx below) */
	unsigned char Type;

} TEDTLVSkipItem;
#define ED_SKIPLIST_OPTIONAL      0
#define ED_SKIPLIST_MANDATORY     1
#define ED_SKIPLIST_END_OF_LIST   2

/*------------------------------------------------------------------------------
	Standard locator setting macro
------------------------------------------------------------------------------*/
/* This macro is used to set a locator. A standard locator is composed by a 32 bit */
/* value where high 16 bit are offset, lower 16 bit are length. */
#ifndef ED_LOCATOR_SET
#define ED_LOCATOR_SET(edLocatorPtr, offsetInBits, lengthInBits) EDStandardLocatorSet (edLocatorPtr, offsetInBits, lengthInBits)
#endif

#ifndef ED_LOCATOR_GET_OFFSET
#define ED_LOCATOR_GET_OFFSET(edLocatorPtr) ((*(edLocatorPtr)) >> 16)
#endif
#ifndef ED_LOCATOR_GET_LENGTH
#define ED_LOCATOR_GET_LENGTH(edLocatorPtr) ((*(edLocatorPtr)) & 0xFFFF)
#endif

#define ED_STANDARD_LOCATOR_INVALID 0
#ifndef ED_LOCATOR_IS_VALID
#define ED_LOCATOR_IS_VALID(edLocatorPtr) ((*edLocatorPtr) != ED_STANDARD_LOCATOR_INVALID)
#endif

/*------------------------------------------------------------------------------
	Standard locator setting function
------------------------------------------------------------------------------*/
extern void EDStandardLocatorSet (unsigned ED_EXLONG* edLocatorPtr, ED_EXLONG offsetInBits, ED_EXLONG lengthInBits);

/*------------------------------------------------------------------------------
	Given a sorted array of TEDKnownIEs, finds the given IE and returns its
	entry or returns NULL if not found.
------------------------------------------------------------------------------*/
const TEDKnownIE* EDFindKnownIE (unsigned char IEI, const TEDKnownIE* KnownIEs, int ArraySize);

/*------------------------------------------------------------------------------
	When decoding, skips the current IE if known. If not known, it returns
	ED_FALSE. Otherwise, it returns ED_TRUE.
	"CurrentOffset" is increased according to the known IE size.
------------------------------------------------------------------------------*/
int EDSkipKnownIE (const void* Source, ED_EXLONG* CurrentOffset, const TEDKnownIE* KnownIEs, int ArraySize);

/*------------------------------------------------------------------------------
	Reads the IEI of an optional IE.
	It receives the list of the possible expected IEIs.
	If the IEI is not one of the possible valid next IEIs, the IE is discarded
	using EDSkipKnownIE function.
	- if it is the expected one, it returns the tag, >=0;
	- if it is an unexpected one, it skips it;
	- if it isn't able to skip it, returns ED_UNKNOWN_IEI;
	- if it goes past the end of the message, it returns ED_MESSAGE_TOO_SHORT.
	- if it meets an IEI on the "expected list" returns ED_MISSING_OPTIONAL.
	Parameters:
	  ExpectedIEI         IEI we expect
		IEISizeInBits       size of the IEI we expect (usually 8 bits, sometimes 4)
		Source              Buffer containing the message
		CurrentOffset       Offset in "Source" where we are expected to start
		                    reading (in bits). This value is updated if some
												IE is skipped.
		EndOfMessage        Offset of the first bit after the end of the message
		KnonwIEs            Pointer to the list of known information elements
		ArraySize           Number of items in the known information elements list
		SkipList            Pointer to a TEDTLVSkipItem array, used to know the
		                    next expected items. NULL if the IE is mandatory.
------------------------------------------------------------------------------*/
int EDGetIE (int ExpectedIEI, int IEISizeInBits, const void* Source, ED_EXLONG* CurrentOffset, ED_EXLONG EndOfMessage, const TEDKnownIE* KnownIEs, int ArraySize, const TEDTLVSkipItem* SkipList);

#ifdef __cplusplus
	}
#endif

#ifndef ED_DISABLE_OUT_OF_MEMORY_CHECK

/* The expression "a" is to be executed only if memory has to be checked */
#ifndef ED_CHECK_OUT_OF_MEMORY_DISCARD
#define ED_CHECK_OUT_OF_MEMORY_DISCARD(a) if (a) return ED_OUT_OF_MEMORY;
#endif

/* The expression "a" is to be executed always, even if memory does not have to be checked */
#ifndef ED_CHECK_OUT_OF_MEMORY_KEEP
#define ED_CHECK_OUT_OF_MEMORY_KEEP(a,b) if ((a) b) return ED_OUT_OF_MEMORY;
#endif

#ifndef ED_CHECK_OUT_OF_MEMORY_IF
#define ED_CHECK_OUT_OF_MEMORY_IF(a,b,c) if ((a) b) {c}
#endif

#ifndef ED_ON_CHECK_OUT_OF_MEMORY
#define ED_ON_CHECK_OUT_OF_MEMORY(a) a
#endif

#else

#ifndef ED_CHECK_OUT_OF_MEMORY_DISCARD
#define ED_CHECK_OUT_OF_MEMORY_DISCARD(a)
#endif

#ifndef ED_CHECK_OUT_OF_MEMORY_KEEP
#define ED_CHECK_OUT_OF_MEMORY_KEEP(a,b) a;
#endif

#ifndef ED_CHECK_OUT_OF_MEMORY_IF
#define ED_CHECK_OUT_OF_MEMORY_IF(a,b,c) a;
#endif

#ifndef ED_ON_CHECK_OUT_OF_MEMORY
#define ED_ON_CHECK_OUT_OF_MEMORY(a)
#endif

#endif

#endif
