/* BSD 2-Clause License



Copyright (c) 2020, yoo.huang_@outlook.com

All rights reserved.



Redistribution and use in source and binary forms, with or without

modification, are permitted provided that the following conditions are met:



1. Redistributions of source code must retain the above copyright notice, this

   list of conditions and the following disclaimer.



2. Redistributions in binary form must reproduce the above copyright notice,

   this list of conditions and the following disclaimer in the documentation

   and/or other materials provided with the distribution.



THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"

AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE

IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE

DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE

FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL

DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR

SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER

CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,

OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE

OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LockFreeQueue.h"
#include <thread>
#include <functional>

using Queue = LockFreeQueue<int, 8, 2048>;

void push(Queue& queue)
{
	int i = 0;
	for (; i < 100000; ++i)
	{
		queue.push(i + 1);
	}
}
void push2(Queue& queue)
{
	int i = 100000;
	for (; i < 200000; ++i)
	{
		queue.push(i + 1);
	}
}
void push3(Queue& queue)
{
	int i = 200000;
	for (; i < 300000; ++i)
	{
		queue.push(i + 1);
	}
}
void push4(Queue& queue)
{
	int i = 300000;
	for (; i < 400000; ++i)
	{
		queue.push(i + 1);
	}
}
void pop(Queue& queue)
{
	int i = 0;
	int n;
	for (; i < 100000; ++i)
	{
		while (!queue.pop(n));
		fprintf(stdout, "%d\n", n);
		Queue::ReclaimLocalHazardNodes();
	}
}

int main()
{
	Queue queue;
	auto pushf = [&queue]() { push(queue); };
	auto pushf2 = [&queue]() { push2(queue); };
	auto pushf3 = [&queue]() { push3(queue); };
	auto pushf4 = [&queue]() { push4(queue); };
	auto popf = [&queue]() { pop(queue); };
	std::thread thd(pushf);
	std::thread thd2(popf);
	std::thread thd6(pushf2);
	std::thread thd7(pushf3);
	std::thread thd8(pushf4);
	std::thread thd3(popf);
	std::thread thd4(popf);
	std::thread thd5(popf);
	thd.join();
	thd2.join();
	Queue::ReclaimHazardNodes();
	thd6.join();
	thd7.join();
	thd8.join();
	thd3.join();
	Queue::ReclaimHazardNodes();
	thd4.join();
	Queue::ReclaimHazardNodes();
	thd5.join();
	Queue::ReclaimHazardNodes();
}
