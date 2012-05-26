/*
 * Copyright (C) 2006  Adam Charrett
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************

atsctext.h

ATSC Text Conversion functions.

*/
#ifndef _ATSCTEXT_H
#define _ATSCTEXT_H
#include <stdint.h>


/**
 * @addtogroup TextConversion Text Conversion
 * @{
 */

/**
 * Structure to describe an ATSC string.
 */
typedef struct ATSCString_s
{
    char lang[3]; /**< ISO639.2/B language code. */
    char *text;   /**< UTF-8 encoded text. */
}ATSCString_t;

/**
 * Structure to describe multiple ATSC strings.
 */
typedef struct ATSCMultipleStrings_s
{
    int number_of_strings; /**< Number of strings pointed to by strings. */
    ATSCString_t *strings; /**< Pointer to the array of strings. */
}ATSCMultipleStrings_t;

/**
 * Converts raw multiple string data into a ATSCMultipleStrings_t structure.
 * @param data Raw multiple string data.
 * @param len Length of the raw data.
 * @return A pointer to a ATSCMultipleStrings_t or NULL.
 * To free the returned structure ObjectRefDec should be used.
 */
#if 0
ATSCMultipleStrings_t *ATSCMultipleStringsConvert(uint8_t *data, uint8_t len);
#else
ATSCMultipleStrings_t *ATSCMultipleStringsConvert(ATSCMultipleStrings_t *result, uint8_t *data, uint8_t len);
#endif

/**
 * @internal
 * Initialise the ATSC multiple strings module.
 * @returns 0 on success.
 */
int ATSCMultipleStringsInit(void);

/** 
 * @internal
 * Deinitiaise the  ATSC multiple strings module.
 */
void ATSCMultipleStringsDeInit(void);
/** @} */
#endif
