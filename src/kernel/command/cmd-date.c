/*
 * kernel/command/cmd-date.c
 *
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <command/command.h>

static void usage(void)
{
	printf("usage:\r\n");
	printf("    date [FORMAT]\r\n");
}

static int do_date(int argc, char ** argv)
{
	struct tm tm;
	char * fmt = "%c";
	char buf[256];
	time_t t;

	if(argc > 1)
		fmt = argv[1];
	if(time(&t) < 0)
		return -1;
	strftime(buf, sizeof(buf), fmt, localtime_r(&t, &tm));
	printf("%s\r\n", buf);
	return 0;
}

static struct command_t cmd_date = {
	.name	= "date",
	.desc	= "print the system date and time",
	.usage	= usage,
	.exec	= do_date,
};

static __init void date_cmd_init(void)
{
	register_command(&cmd_date);
}

static __exit void date_cmd_exit(void)
{
	unregister_command(&cmd_date);
}

command_initcall(date_cmd_init);
command_exitcall(date_cmd_exit);
