/*
 *  user_strings_atari.cpp - Localizable strings, Atari implementation
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"
#include "user_strings.h"


// Platform-specific string definitions
user_string_def platform_strings[] = {
	// Common strings that have a platform-specific variant
	{STR_VOLUME_IS_MOUNTED_WARN, "The volume '%s' is mounted on Atari. Basilisk II will try to unmount it."},
	{STR_EXTFS_CTRL, "Atari Root"},
	{STR_EXTFS_NAME, "Atari Directory Tree"},
	{STR_EXTFS_VOLUME_NAME, "Atari"},

	{-1, (const char*) NULL}	// End marker
};


/*
 *  Fetch pointer to string, given the string number
 */

const char *GetString(int num)
{
	// First search for platform-specific string
	int i = 0;
	while (platform_strings[i].num >= 0) {
		if (platform_strings[i].num == num)
			return platform_strings[i].str;
		i++;
	}

	// Not found, search for common string
	i = 0;
	while (common_strings[i].num >= 0) {
		if (common_strings[i].num == num)
			return common_strings[i].str;
		i++;
	}
	return (const char*) NULL;
}
