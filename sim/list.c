/**
 * \addtogroup list
 * @{
 */

/**
 * \file
 * Linked list library implementation.
 *
 * \author Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: list.c,v 1.1 2006/06/17 22:41:18 adamdunkels Exp $
 */
#include "list.h"

/**
 * Initialize a list.
 *
 * This function initalizes a list. The list will be empty after this
 * function has been called.
 *
 * \param list The list to be initialized.
 */
void list_init(struct list_head *list)
{
	list->next = NULL;
}

/**
 * Get a pointer to the first element of a list.
 *
 * This function returns a pointer to the first element of the
 * list. The element will \b not be removed from the list.
 *
 * \param list The list.
 * \return A pointer to the first element on the list.
 *
 * \sa list_tail()
 */
struct list_head *list_head(struct list_head *list)
{
	return list->next;
}

/**
 * Duplicate a list.
 *
 * This function duplicates a list by copying the list reference, but
 * not the elements.
 *
 * \note This function does \b not copy the elements of the list, but
 * merely duplicates the pointer to the first element of the list.
 *
 * \param dest The destination list.
 * \param src The source list.
 */
void list_copy(struct list_head *dest, struct list_head *src)
{
	dest->next = src->next;
}

/**
 * Get the tail of a list.
 *
 * This function returns a pointer to the elements following the first
 * element of a list. No elements are removed by this function.
 *
 * \param list The list
 * \return A pointer to the element after the first element on the list.
 *
 * \sa list_head()
 */
struct list_head *list_tail(struct list_head *list)
{
	struct list_head *l = list;

	for(; l->next != NULL; l = l->next);

	return l;
}

/*
 * Add an item at the end of a list.
 *
 * This function adds an item to the end of the list.
 *
 * \param list The list.
 * \param item A pointer to the item to be added.
 *
 * \sa list_push()
 *
 */
void list_add(struct list_head *list, struct list_head *item)
{
	struct list_head *l;

	item->next = NULL;
	l = list_tail(list);
	l->next = item;
}

/*
 * Add an item to the start of the list.
 */
void list_push(struct list_head *list, struct list_head *item)
{
	item->next = list;
	list->next = item;
}

/*
 * Remove the last object on the list.
 *
 * This function removes the last object on the list and returns it.
 *
 * \param list The list
 * \return The removed object
 *
 */
struct list_head *list_chop(struct list_head *list)
{
	struct list_head *l, *r;

	if(list->next == NULL)
		return NULL;

	if(list->next->next == NULL) {
		l = list;
		list = NULL;
		return l;
	}

	for(l = list->next; l->next->next != NULL; l = l->next);

	r = l->next;
	l->next = NULL;

	return r;
}

/**
 * Remove the first object on a list.
 *
 * This function removes the first object on the list and returns a
 * pointer to the list.
 *
 * \param list The list.
 * \return The new head of the list.
 */
struct list_head *list_pop(struct list_head *list)
{
	if(list->next != NULL)
		list = list->next;

	return list;
}

/**
 * Remove a specific element from a list.
 *
 * This function removes a specified element from the list.
 *
 * \param list The list.
 * \param item The item that is to be removed from the list.
 *
 */
void list_remove(struct list_head *list,
		 struct list_head *item)
{
	struct list_head *l, *r;

	if(list->next == NULL)
		return;

	r = list;
	for(l = list->next; l != NULL; l = l->next) {
		if(l == item) {
			/* Not first on list */
			r->next = l->next;
			l->next = NULL;
			return;
		}
		r = l;
	}
}

/**
 * Get the length of a list.
 *
 * This function counts the number of elements on a specified list.
 *
 * \param list The list.
 * \return The length of the list.
 */
int list_length(struct list_head *list)
{
	struct list_head *l;
	int n = 0;

	for(l = list->next; l != NULL; l = l->next) {
		++n;
	}

	return n;
}

/**
 * \brief      Insert an item after a specified item on the list
 * \param list The list
 * \param previtem The item after which the new item should be inserted
 * \param newitem  The new item that is to be inserted
 * \author     Adam Dunkels
 *
 *             This function inserts an item right after a specified
 *             item on the list. This function is useful when using
 *             the list module to ordered lists.
 *
 *             If previtem is NULL, the new item is placed at the
 *             start of the list.
 *
 */
void list_insert(struct list_head *list,
		 struct list_head *previtem,
		 struct list_head *newitem)
{
	if(previtem == NULL) {
		list_push(list, newitem);
	} else {
		newitem->next = previtem->next;
		previtem->next = newitem;
	}
}
