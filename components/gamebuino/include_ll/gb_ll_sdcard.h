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
#ifdef __cplusplus
extern "C" {
#endif

#include "gb_err.h"
#include <stdbool.h>

/**
 * Mount FAT on MOUNT_POINT ("/sdcard") via SDMMC.
 * Never formats the card (format_if_mount_failed = false).
 *
 * @return GB_OK on success, GB_ERR_IO if the card/filesystem cannot be mounted,
 *         GB_ERR_BUSY if already mounted.
 */
int gb_ll_sd_init(void);

/** True after a successful gb_ll_sd_init(). */
bool gb_ll_sd_is_mounted(void);

#ifdef __cplusplus
}
#endif