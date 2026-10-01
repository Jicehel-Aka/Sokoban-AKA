/*
This file is part of the Gamebuino-AKA library,
Copyright (c) Gamebuino 2026

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License (LGPL) for more details.

You should have received a copy of the GNU Lesser General Public
License (LGPL) along with the library.
If not, see <http://www.gnu.org/licenses/>.

Authors:
 - Jean-Marie Papillon
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Documented integer error codes for the gamebuino component and pAKAman core.
 *
 * Convention used by every public function that reports success/failure:
 *   GB_OK (0)  on success
 *   negative   on failure (see macros below)
 *
 * Callers must not treat any non-zero value as success. Logging (ESP_LOG /
 * printf) may accompany the code; the return value is the API contract.
 */
#define GB_OK               0
#define GB_ERR              (-1)  /**< generic / unspecified failure */
#define GB_ERR_PARAM        (-2)  /**< null pointer, empty path, out of range */
#define GB_ERR_IO           (-3)  /**< I2C, I2S, ADC, or file I/O failure */
#define GB_ERR_FORMAT       (-4)  /**< unsupported or corrupt file (WAV, etc.) */
#define GB_ERR_NO_SPACE     (-5)  /**< no free audio slot, or write incomplete */
#define GB_ERR_NOT_FOUND    (-6)  /**< object / file not present */
#define GB_ERR_BUSY         (-7)  /**< resource already in use */
#define GB_ERR_NOT_MOUNTED  (-8)  /**< SD filesystem not mounted */
#define GB_ERR_OVERFLOW     (-9)  /**< size, index, or buffer bounds exceeded */

#ifdef __cplusplus
}
#endif
