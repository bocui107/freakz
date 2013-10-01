/*******************************************************************
    Copyright (C) 2008 FreakLabs

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Originally written by Christopher Wang.
    Please post support questions to the FreakLabs forum.
*******************************************************************/
/*!
    \file mem_pool.c

    Allocate and free entries from the various data struct pools in the
    stack.
*/
#include "freakz.h"

struct entry
{
	struct entry *next;
	bool alloc;
};

/*
 * Generic allocation function to allocate a structure from a struct pool. It
 * requires entering the pool pointer (array head), pool size (array size), and
 * the size of an individual struct element.
 */
void *mem_pool_alloc(void *pool, U8 pool_size, U8 entry_size)
{
	U8 i;
	struct entry *item;
	U8 *pool_ptr;

	pool_ptr = pool;

	for (i = 0; i < pool_size; i++)
	{
		item = (struct entry *)pool_ptr;
		if (!item->alloc) {
			memset(item, 0, entry_size);
			item->alloc = true;
			return item;
		}
		pool_ptr += entry_size;
	}
	return NULL;
}

/* Generic function to free an entry back into a struct pool */
void mem_pool_free(void *entry)
{
	struct entry *item;

	if (entry) {
		item = (struct entry *)entry;
		item->alloc = false;
	}
}
