/*
 * kernel/xui/xui-panel.c
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
#include <xui/panel.h>

void xui_begin_panel_ex(struct xui_context_t * ctx, const char * name, int opt)
{
	struct xui_container_t * c;
	struct color_t * bg, * bc;
	struct region_t * r;

	xui_push_id(ctx, name, strlen(name));
	c = get_container(ctx, ctx->last_id, opt & ~XUI_OPT_CLOSED);
	r = xui_layout_next(ctx);
	region_clone(&c->region, r);
	if(~opt & XUI_PANEL_TRANSPARENT)
	{
		bg = &ctx->style.panel.background_color;
		bc = &ctx->style.panel.border_color;
		int br = ctx->style.panel.border_radius;
		int bw = ctx->style.panel.border_width;
		int sr = ctx->style.panel.shadow_radius;
		if(sr > 0)
			xui_draw_shadow(ctx, r->x, r->y, r->w, r->h, max(sr, br), &(struct color_t){0, 0, 0, 255}, 0);
		if(bc->a && (bw > 0))
			xui_draw_rectangle(ctx, r->x, r->y, r->w, r->h, br, bw, bc);
		if(bg->a)
			xui_draw_rectangle(ctx, r->x, r->y, r->w, r->h, br, 0, bg);
	}
	scroll_begin(ctx, c, opt);
	xui_push(ctx->container_stack, c);
	push_container_body(ctx, c, &c->region);
	xui_push_clip(ctx, &c->body);
}

void xui_end_panel(struct xui_context_t * ctx)
{
	struct xui_container_t * c = xui_get_container(ctx);
	scroll_end(ctx, c);
	xui_pop_clip(ctx);
	pop_container(ctx);
}
