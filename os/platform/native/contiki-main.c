/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the Contiki OS
 *
 * $Id: contiki-main.c,v 1.6 2007/11/28 12:54:42 adamdunkels Exp $
 *
 */

#include <stdio.h>
#include <sys/select.h>
#include "contiki.h"
#include "freakz.h"

PROCINIT(&etimer_process);

int contiki_main(void)
{
	int n;
	struct timeval tv;

	process_init();
	procinit_init();

#if (TEST_SIM == 0)
	autostart_start((struct process **)autostart_processes);
#endif

	printf("Contiki initiated, now starting process scheduling\n");

	freakz_init();

	while(1)
	{
		n = process_run();
		tv.tv_sec = 0;
		tv.tv_usec = 1;
		/*
		 * int select(int n,
		 *	      fd_set *readfds,
		 *	      fd_set *writefds,
		 *	      fd_set *exceptfds,
		 *	      struct timeval *timeout);
		 *
		 * select() wait the change of the file.
		 * n: the max file describe + 1
		 * readfds, writefds and exceptfds is the set of the file,
		 *
		 * struct timeval *timeout是select的超时时间, 这个参数至关重要,
		 * 它可以使select处于三种状态:
		 * 第一. 若将NULL以形参传入, 即不传入时间结构, 就是将select置于阻塞状态,
		 * 一定等到监视文件描述符集合中某个文件描述符发生变化为止;
		 *
		 * 第二. 若将时间值设为0秒0毫秒, 就变成一个纯粹的非阻塞函数,
		 * 不管文件描述符是否有变化, 都立刻返回继续执行, 文件无变化返回0,
		 * 有变化返回一个正值;
		 *
		 * 第三. timeout的值大于0, 这就是等待的超时时间, 即select在timeout时间内阻塞,
		 * 超时时间之内有事件到来就返回了, 否则在超时后不管怎样一定返回, 返回值同上述
		 */
		select(0, NULL, NULL, NULL, &tv);
		etimer_request_poll();
	}

	return 0;
}

void log_message(char *m1, char *m2)
{
	printf("%s%s\n", m1, m2);
}
