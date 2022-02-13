
#include <xdk.h>
#include <xdu.h>
/*
void test_blut()
{
    if_blut_t if_blut = { 0 };
    
    xdu_impl_blut(&if_blut);

	if_socket_t if_sock = { 0 };

	xdk_impl_socket(&if_sock);

	(*if_sock.pf_socket_startup)();

    int n = (*if_blut.pf_enum_blut)(NULL, MAX_LONG);

    dev_blt_t* pdb = (dev_blt_t*)calloc(n, sizeof(dev_blt_t));

    (*if_blut.pf_enum_blut)(pdb, n);

    async_t asy = {0};
    asy.type = ASYNC_BLOCK;
    asy.timo = INFINITE;

    byte_t ch[1];
    for(int i = 0;i<n;i++)
    {
        printf("%s %s %s %s \n", pdb[i].addr, pdb[i].name, pdb[i].major_class, pdb[i].minor_class);

        res_file_t fd = (*if_blut.pf_blut_open)(pdb[i].addr, 0, 0);
        if(fd != INVALID_FILE)
        {
            (*if_blut.pf_blut_read)(fd, &ch, 1, &asy);
            (*if_blut.pf_blut_close)(fd);
        }
    }

    free(pdb);

	(*if_sock.pf_socket_cleanup)();
}
*/
/**************************************************************************/

if_context_t if_context = {0};
if_widget_t if_widget = {0};
res_win_t g_main = NULL;

int pop_on_create(res_win_t wt, void* param)
{
    printf("pop on_create\n");

    (*if_widget.pf_widget_post_command)(wt, 4, IDC_SELF, 100);
    
    return 0;
}

void pop_on_destroy(res_win_t wt)
{
    printf("pop on_destroy\n");
}

void pop_on_mouse_move(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("pop on_mouse_move x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("pop on_mouse_move x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("pop on_mouse_move x:%d y:%d\n", ppt->x, ppt->y);
}

void pop_on_mouse_hover(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("pop on_mouse_hover x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("pop on_mouse_hover x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("pop on_mouse_hover x:%d y:%d\n", ppt->x, ppt->y);
}

void pop_on_mouse_leave(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("pop on_mouse_leave x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("pop on_mouse_leave x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("pop on_mouse_leave x:%d y:%d\n", ppt->x, ppt->y);
}

void pop_on_lbutton_down(res_win_t wt, const xpoint_t* ppt)
{
    printf("pop on_lbutton_down x:%d y:%d\n", ppt->x, ppt->y);

    res_win_t par =  (*if_widget.pf_widget_get_owner)(wt);

    (*if_widget.pf_widget_send_command)(par, 1, 1000, 100);
}

void pop_on_lbutton_up(res_win_t wt, const xpoint_t* ppt)
{
    printf("pop on_lbutton_up x:%d y:%d\n", ppt->x, ppt->y);

    res_win_t par =  (*if_widget.pf_widget_get_parent)(wt);

    (*if_widget.pf_widget_post_command)(par, 2, IDC_CHILD, 100);

    (*if_widget.pf_widget_destroy)(wt);
}

void pop_on_self_command(res_win_t wt, int code, vword_t data)
{
    printf("pop on_self_command code: %d \n", code);
}
/****************************************************************************/
int child_on_create(res_win_t wt, void* param)
{
    printf("child on_create\n");

    return 0;
}

void child_on_destroy(res_win_t wt)
{
    printf("child on_destroy\n");
}

void child_on_mouse_move(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("child on_mouse_move x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("child on_mouse_move x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("child on_mouse_move x:%d y:%d\n", ppt->x, ppt->y);
}

void child_on_mouse_hover(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("child on_mouse_hover x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("child on_mouse_hover x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("child on_mouse_hover x:%d y:%d\n", ppt->x, ppt->y);
}

void child_on_mouse_leave(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("child on_mouse_leave x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("child on_mouse_leave x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("child on_mouse_leave x:%d y:%d\n", ppt->x, ppt->y);
}

void child_on_lbutton_down(res_win_t wt, const xpoint_t* ppt)
{
    printf("child on_lbutton_down x:%d y:%d\n", ppt->x, ppt->y);

    (*if_widget.pf_widget_set_focus)(wt);

    res_win_t par =  (*if_widget.pf_widget_get_parent)(wt);

    NOTICE nt = {0};

    nt.id = 1000;
    nt.code = 5;
    nt.widget = wt;

    int n = (*if_widget.pf_widget_send_notice)(par, &nt);

    printf("child notice return %d \n", n);
    
    nt.id = 1000;
    nt.code = 6;
    nt.widget = wt;

   (*if_widget.pf_widget_send_notice)(par, &nt);

   (*if_widget.pf_widget_set_capture)(wt, 1);
}

void child_on_lbutton_up(res_win_t wt, const xpoint_t* ppt)
{
    printf("child on_lbutton_up x:%d y:%d\n", ppt->x, ppt->y);

    (*if_widget.pf_widget_set_capture)(wt, 0);

    (*if_widget.pf_widget_post_char)(0, 'A');

    (*if_widget.pf_widget_erase)(wt, NULL);
}

void child_on_lbutton_dbclick(res_win_t wt, const xpoint_t* ppt)
{
    printf("child on_lbutton_dbclick x:%d y:%d\n", ppt->x, ppt->y);
}

void child_on_rbutton_up(res_win_t wt, const xpoint_t* ppt)
{
    printf("child on_rbutton_up x:%d y:%d\n", ppt->x, ppt->y);

     if_event_t ev = {0};

    ev.pf_on_create = pop_on_create;
	ev.pf_on_destroy = pop_on_destroy;
    ev.pf_on_mouse_move = pop_on_mouse_move;
	ev.pf_on_mouse_hover = pop_on_mouse_hover;
	ev.pf_on_mouse_leave = pop_on_mouse_leave;
    ev.pf_on_lbutton_down = pop_on_lbutton_down;
    ev.pf_on_lbutton_up = pop_on_lbutton_up;
    ev.pf_on_self_command = pop_on_self_command;

    xrect_t xr;
    xr.x = ppt->x;
    xr.y = ppt->y;
    xr.w = 200;
    xr.h = 300;

    (*if_widget.pf_widget_client_to_screen)(wt, RECTPOINT(&xr));

    res_win_t pop = (*if_widget.pf_widget_create)(_T("popup"),WD_STYLE_MENU,&xr,g_main,&ev);

    (*if_widget.pf_widget_show)(pop, 0);

    (*if_widget.pf_widget_set_owner)(pop, wt);

    (*if_widget.pf_widget_take)(pop, WS_TAKE_TOPMOST);

    (*if_widget.pf_widget_do_trace)(pop);
}

void child_on_keydown(res_win_t wt, dword_t ks, int key)
{
    if(ks & KS_WITH_CONTROL)
        printf("child on_key_down keycode:%02x with control\n", key);
    else if(ks & KS_WITH_SHIFT)
        printf("child on_key_down keycode:%02x with shift\n", key);
    else
        printf("child on_key_down keycode:%02x\n", key);
}

void child_on_set_focus(res_win_t wt, res_win_t from)
{
    printf("child on_set_focus \n");

    (*if_widget.pf_widget_create_caret)(wt, 100, 100);
}

void child_on_kill_focus(res_win_t wt, res_win_t to)
{
    printf("child on_kill_focus \n");

    (*if_widget.pf_widget_destroy_caret)(wt);
}

void child_on_menu_command(res_win_t wt, int code, int cid, vword_t data)
{
    printf("child on_menu_command code: %d cid: %d \n", code, cid);
}

void child_on_parent_command(res_win_t, int code, vword_t data)
{
    printf("child on_parent_command code: %d \n", code);
}

void child_on_paint(res_win_t wt, visual_t rdc, const xrect_t* prt)
{
    printf("child on_paint the rect is x:%d y:%d w:%d h:%d \n", prt->x, prt->y, prt->w, prt->h);

    xrect_t rt;
     (*if_widget.pf_widget_get_client_rect)(wt, &rt);
    visual_t ctx = (*if_context.pf_create_compatible_context)(rdc, rt.w, rt.h);

    xpen_t xp = {0};
    default_xpen(&xp);
    xscpy(xp.color, GDI_ATTR_RGB_GREEN);

    xbrush_t xb = {0};
    default_xbrush(&xb);
    xscpy(xb.color, GDI_ATTR_RGB_GRAY);

    xrect_t xr;
    xr.x = 0;
    xr.y = 0;
    xr.w = rt.w;
    xr.h = rt.h;

    xscpy(xb.color, GDI_ATTR_RGB_SOFTBLACK);
    (*if_context.pf_gdi_draw_rect)(ctx, &xp, &xb, &xr);

    xpoint_t xp1,xp2;
    xp1.x = 50;
    xp1.y = 50;
    xp2.x = 80;
    xp2.y = 20;

    xscpy(xp.color, GDI_ATTR_RGB_SLATE);
   //(*if_context.pf_gdi_draw_line)(ctx, &xp, &xp1, &xp2);

    xsize_t xs1;
    xs1.w = 30;
    xs1.h = 30;

    xscpy(xp.size,_T("1"));
    xscpy(xp.color, GDI_ATTR_RGB_YELLOW);
   //(*if_context.pf_gdi_draw_arc)(ctx, &xp, &xp1, &xp2, &xs1, 0, 0);

   xcolor_t brim_color, core_color;
   parse_xcolor(&brim_color, GDI_ATTR_RGB_RED);
   parse_xcolor(&core_color, GDI_ATTR_RGB_BLUE);

    xr.x = 2;
    xr.y = 2;
    xr.w = 96;
    xr.h = 96;
  // (*if_context.pf_gdi_gradient_rect)(ctx, &brim_color, &core_color, GDI_ATTR_GRADIENT_RADIAL, &xr);

   xr.x = 5;
   xr.y = 5;
   xr.w = 90;
   xr.h = 90;

    xscpy(xb.color, GDI_ATTR_RGB_SOFTRED);
   // (*if_context.pf_gdi_draw_round)(ctx, &xp, &xb, &xr);

   xcolor_t xc = { 0 };
   parse_xcolor(&xc, GDI_ATTR_RGB_GREEN);
   //(*if_context.pf_gdi_alphablend_rect)(ctx, &xc, &xr, 64);

    xr.x = 50;
    xr.y = 50;
    xr.w = 32;
    xr.h = 32;
    xscpy(xb.color, GDI_ATTR_RGB_SLATE);
  // (*if_context.pf_gdi_draw_ellipse)(ctx, &xp, &xb, &xr);

    xscpy(xp.color, GDI_ATTR_RGB_GREEN);
    xscpy(xb.color, GDI_ATTR_RGB_BLUE);
  //(*if_context.pf_gdi_draw_pie)(ctx, &xp, &xb, &xr, XPI / 4, XPI / 2);

    xr.x = 50;
    xr.y = 50;
    xr.w = 32;
    xr.h = 32;
    xscpy(xb.color, GDI_ATTR_RGB_SLATE);
    //(*if_context.pf_gdi_draw_triangle)(ctx, &xp, &xb, &xr, GDI_ATTR_ORIENT_BOTTOM);

	xp1.x = 40;
	xp1.y = 40;

	xspan_t s1, s2;
	s1.r = 50;
	s2.r = 30;
	xscpy(xb.color, GDI_ATTR_RGB_SLATE);
	//(*if_context.pf_gdi_draw_sector)(ctx, &xp, &xb, &xp1, &s1, &s2, XPI / 4,  XPI);

    xpoint_t pt[5];
    pt[0].x = 40;
    pt[0].y = 40;
    pt[1].x = 100;
    pt[1].y = 60;
    pt[2].x = 80;
    pt[2].y = 80;
    pt[3].x = 20;
    pt[3].y = 60;
    pt[4].x = 40;
    pt[4].y = 40;
    xscpy(xp.color, GDI_ATTR_RGB_BLUE);
    //(*if_context.pf_gdi_draw_polyline)(ctx, &xp, pt, 4);

    xscpy(xb.color, GDI_ATTR_RGB_SLATE);
    //(*if_context.pf_gdi_draw_polygon)(ctx, &xp, &xb, pt, 4);

    xscpy(xp.color, GDI_ATTR_RGB_SLATE);
    //(*if_context.pf_gdi_draw_bezier)(ctx, &xp, &pt[0], &pt[1], &pt[2], &pt[3]);
	xscpy(xp.color, GDI_ATTR_RGB_RED);
	//(*if_context.pf_gdi_draw_curve)(ctx, &xp, pt, 4);

	tchar_t aa[10] = { 0 };
	xpoint_t pa[20] = { 0 };

	int i = 0;
	int n = 0;
	int feed = 3;

	xr.x = 2;
	xr.y = 2;
	xr.w = 96;
	xr.h = 20;

	aa[i] = _T('M');
	pa[n].x = xr.x;
	pa[n].y = xr.y + feed;
	i++;
	n++;

	aa[i] = _T('A');
	pa[n].x = 1;
	pa[n].y = 0;
	pa[n + 1].x = feed;
	pa[n + 1].y = feed;
	pa[n + 2].x = xr.x + feed;
	pa[n + 2].y = xr.y;
	i++;
	n += 3;

	aa[i] = _T('L');
	pa[n].x = xr.x + xr.w - feed;
	pa[n].y = xr.y;
	i++;
	n++;

	aa[i] = _T('A');
	pa[n].x = 1;
	pa[n].y = 0;
	pa[n + 1].x = feed;
	pa[n + 1].y = feed;
	pa[n + 2].x = xr.x + xr.w;
	pa[n + 2].y = xr.y + feed;
	i++;
	n += 3;

	aa[i] = _T('L');
	pa[n].x = xr.x + xr.w;
	pa[n].y = xr.y + xr.h - feed;
	i++;
	n++;

	aa[i] = _T('A');
	pa[n].x = 1;
	pa[n].y = 0;
	pa[n + 1].x = feed;
	pa[n + 1].y = feed;
	pa[n + 2].x = xr.x + xr.w - feed;
	pa[n + 2].y = xr.y + xr.h;
	i++;
	n += 3;

	aa[i] = _T('C');
	pa[n].x = xr.x + xr.w / 8 * 7;
	pa[n].y = xr.y + xr.h - 10;
	pa[n + 1].x = xr.x + xr.w / 4 * 3;
	pa[n + 1].y = xr.y + xr.h - 10;
	pa[n + 2].x = xr.x + xr.w / 2;
	pa[n + 2].y = xr.y + xr.h;
	i++;
	n += 3;

	aa[i] = _T('S');
	pa[n].x = xr.x + xr.w / 4;
	pa[n].y = xr.y + xr.h;
	pa[n + 1].x = xr.x + feed;
	pa[n + 1].y = xr.y + xr.h;
	i++;
	n += 2;

	aa[i] = _T('A');
	pa[n].x = 1;
	pa[n].y = 0;
	pa[n + 1].x = feed;
	pa[n + 1].y = feed;
	pa[n + 2].x = xr.x;
	pa[n + 2].y = xr.y + xr.h - feed;
	i++;
	n += 3;

	aa[i] = _T('Z');
	i++;

	//(*if_context.pf_gdi_draw_path)(ctx, &xp, &xb, aa, pa);

    xfont_t xf = {0};
    default_xfont(&xf);
    xscpy(xf.size,_T("12"));
    xscpy(xf.color,GDI_ATTR_RGB_LIGHTCYAN);
    xface_t xa = {0};
    default_xface(&xa);

    xsize_t xs = {0};
    (*if_context.pf_gdi_text_metric)(ctx, &xf, &xs);

   (*if_context.pf_gdi_text_rect)(ctx, &xf, &xa, _T("Hello World!"), -1, &xr);
   xr.x = 10;
   xr.y = 20;
    //(*if_context.pf_gdi_draw_rect)(ctx, &xp, &xb, &xr);

    xr.x = 10;
    xr.y = 20;
    xr.w = 100;
    xr.h = 20;
    //(*if_context.pf_gdi_draw_text)(ctx, &xf, &xa, &xr, _T("Hello World!"), -1);

    //(*if_context.pf_gdi_text_size)(ctx, &xf,  _T("Hello World!"), -1, &xs);

    xp1.x = 0;
    xp1.y = 30;
    xp2.x = xp1.x + xs.w;
    xp2.y = 30;

    (*if_context.pf_gdi_draw_line)(ctx, &xp, &xp1, &xp2);
    (*if_context.pf_gdi_text_out)(ctx, &xf, &xp1, _T("Hello World!"), -1);

    parse_xcolor(&xc, GDI_ATTR_RGB_RED);
    bitmap_t bmp = (*if_context.pf_create_color_bitmap)(ctx, &xc, 32, 32);
    if(bmp)
    {
        xr.x = 10;
        xr.y = 50;
        xr.w = 50;
        xr.h = 50;

        (*if_context.pf_gdi_exclip_rect)(ctx, &xr);

       //(*if_context.pf_gdi_draw_bitmap)(ctx, bmp, RECTPOINT(&xr));
    
        (*if_context.pf_destroy_bitmap)(bmp);
    }

    bmp = (*if_context.pf_create_storage_bitmap)(ctx, _T("./title.jpg"));
    if(bmp)
    {
        xr.x = 10;
        xr.y = 10;
        xr.w = 50;
        xr.h = 50;
       // (*if_context.pf_gdi_draw_bitmap)(ctx, bmp, RECTPOINT(&xr));
       //(*if_context.pf_gdi_draw_image)(ctx, bmp, GDI_ATTR_RGB_AZURE, &xr);

        (*if_context.pf_destroy_bitmap)(bmp);
    }

    (*if_context.pf_render_context)(ctx, 0,0, rdc, 100, 100, 100, 100);

    (*if_context.pf_destroy_context)(ctx);

}
/**********************************************************************************/

int main_on_create(res_win_t wt, void* param)
{
    printf("on_create: main\n");

    if_event_t ev = {0};

    ev.pf_on_create = child_on_create;
	ev.pf_on_destroy = child_on_destroy;
    ev.pf_on_mouse_move = child_on_mouse_move;
	ev.pf_on_mouse_hover = child_on_mouse_hover;
	ev.pf_on_mouse_leave = child_on_mouse_leave;
    ev.pf_on_lbutton_down = child_on_lbutton_down;
    ev.pf_on_lbutton_up = child_on_lbutton_up;
	ev.pf_on_lbutton_dbclick = child_on_lbutton_dbclick;
	ev.pf_on_rbutton_up = child_on_rbutton_up;
	ev.pf_on_keydown = child_on_keydown;
	ev.pf_on_set_focus = child_on_set_focus;
	ev.pf_on_kill_focus = child_on_kill_focus;
	ev.pf_on_paint = child_on_paint;
	ev.pf_on_menu_command = child_on_menu_command;
	ev.pf_on_parent_command = child_on_parent_command;

	xrect_t xr = { 0 };
    xr.x = 50;
	xr.y = 50;
	xr.w = 400;
	xr.h = 300;

    res_win_t child = (*if_widget.pf_widget_create)(_T("child"),WD_STYLE_CHILD,&xr,wt,&ev);

    (*if_widget.pf_widget_set_user_id)(child, 1000);

    (*if_widget.pf_widget_show)(child, WS_SHOW_NORMAL);

   // (*if_widget.pf_widget_enable)(child, 0);

    return 0;
}

int main_on_close(res_win_t wt)
{
    printf("main on_closein\n");

    return 0;
}

void main_on_destroy(res_win_t wt)
{
    printf("main on_destroy\n");

    res_win_t child = (*if_widget.pf_widget_get_child)(wt, 1000);

    (*if_widget.pf_widget_destroy)(child);
}

void main_on_lbutton_down(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_lbutton_down x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_lbutton_up(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_lbutton_up x:%d y:%d\n", ppt->x, ppt->y);

    res_win_t cld = (*if_widget.pf_widget_get_child)(wt, 1000);
    if(cld)
    {
     (*if_widget.pf_widget_post_command)(cld, 3, IDC_PARENT, 100);
    }
}

void main_on_lbutton_dbclick(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_lbutton_dbclick x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_rbutton_down(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_rbutton_down x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_rbutton_up(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_rbutton_up x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_mouse_move(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("main on_mouse_move x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("main on_mouse_move x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("main on_mouse_move x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_mouse_hover(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("main on_mouse_hover x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("main on_mouse_hover x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("main on_mouse_hover x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_mouse_leave(res_win_t wt, dword_t ms, const xpoint_t* ppt)
{
    if(ms & MS_WITH_LBUTTON)
        printf("main on_mouse_leave x:%d y:%d with lbutton\n", ppt->x, ppt->y);
    else if(ms & MS_WITH_RBUTTON)
        printf("main on_mouse_leave x:%d y:%d with rbutton\n", ppt->x, ppt->y);
    else
        printf("main on_mouse_leave x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_whell(res_win_t wt, bool_t horz, int delta)
{
    if(horz)
        printf("main on_whell_horz delta:%d\n", delta);
    else
       printf("main on_whell_vert delta:%d\n", delta);

    (*if_widget.pf_widget_scroll)(g_main, horz, delta);
}

void main_on_scroll(res_win_t wt, bool_t horz, int pos)
{
    if(horz)
        printf("main on_scroll_horz position:%d\n", pos);
    else
       printf("main on_scroll_vert position:%d\n", pos);
}

void main_on_keydown(res_win_t wt, dword_t ks, int key)
{
    if(ks & KS_WITH_CONTROL)
        printf("main on_key_down keycode:%02x with control\n", key);
    else if(ks & KS_WITH_SHIFT)
        printf("main on_key_down keycode:%02x with shift\n", key);
    else
        printf("main on_key_down keycode:%02x\n", key);
}

void main_on_keyup(res_win_t wt, dword_t ks, int key)
{
    if(ks & KS_WITH_CONTROL)
        printf("main on_key_up keycode:%02x with control\n", key);
    else if(ks & KS_WITH_SHIFT)
        printf("main on_key_up keycode:%02x with shift\n", key);
    else
        printf("main on_key_up keycode:%02x\n", key);
}

void main_on_char(res_win_t wt, tchar_t ch)
{
    printf("main on_char keycode:%c\n", ch);
}

void main_on_size(res_win_t wt, int type, const xsize_t* pst)
{
    if(type == WS_SIZE_MAXIMIZED)
        printf("main on_size maximized width:%d height:%d\n", pst->w, pst->h);
    else if(type == WS_SIZE_MINIMIZED)
        printf("main on_size minimized width:%d height:%d\n", pst->w, pst->h);
    else if(type == WS_SIZE_LAYOUT)
        printf("main on_size layout width:%d height:%d\n", pst->w, pst->h);
    else
        printf("main on_size restore width:%d height:%d\n", pst->w, pst->h);

	res_win_t child = (*if_widget.pf_widget_get_child)(wt, 1000);
	if (child)
	{
		xrect_t xr;
		xr.x = 50;
		xr.y = 50;
		xr.w = 400;
		xr.h = 300;
		(*if_widget.pf_widget_move)(child, RECTPOINT(&xr));
		(*if_widget.pf_widget_size)(child, RECTSIZE(&xr));
	}
}

void main_on_move(res_win_t wt, const xpoint_t* ppt)
{
    printf("main on_move position x:%d y:%d\n", ppt->x, ppt->y);
}

void main_on_show(res_win_t wt, bool_t show)
{
    if(show)
        printf("main on_show \n");
    else
        printf("main on_hide \n");
}

void main_on_activate(res_win_t wt, int ma)
{
    if(ma == WS_ACTIVE_CLICK)
        printf("main on_activate by mouse click \n");
    else
        printf("main on_activate by application \n");
}

void main_on_paint(res_win_t wt, visual_t rdc, const xrect_t* prt)
{
    printf("main main on_paint the rect is x:%d y:%d w:%d h:%d \n", prt->x, prt->y, prt->w, prt->h);
}

void main_on_set_focus(res_win_t wt, res_win_t from)
{
    printf("main on_set_focus \n");
}

void main_on_kill_focus(res_win_t wt, res_win_t to)
{
    printf("main on_kill_focus \n");
}

void main_on_notice(res_win_t, NOTICE* pnt)
{
    printf("main on_notice child id: %d code: %d \n", pnt->id, pnt->code);
}

void main_on_menu_command(res_win_t wt, int code, int cid, vword_t data)
{
    printf("main on_menu_command child id: %d code: %d \n", cid, code);
}

void main_on_parent_command(res_win_t, int code, vword_t data)
{
    printf("main on_parent_command code: %d \n", code);
}

void main_on_child_command(res_win_t wt, int code, vword_t data)
{
    printf("main on_child_command code: %d \n", code);
}

void main_on_self_command(res_win_t wt, int code, vword_t data)
{
    printf("main on_self_command code: %d \n", code);
}

void main_on_command_find(res_win_t, str_find_t*)
{
}

void main_on_comamnd_replace(res_win_t, str_replace_t*)
{
}

void main_on_syscmd_click(res_win_t, const xpoint_t*)
{
}

void main_on_timer(res_win_t, vword_t)
{
}

void main_on_nc_paint(res_win_t wt, visual_t dc)
{
    printf("main on_nc_paint\n");

	border_t bd = { 0 };
    dword_t style;

    style = (*if_widget.pf_widget_get_style)(wt);

	(*if_widget.pf_widget_get_border)(wt, &bd);

    xpen_t xp;
    default_xpen(&xp);
    xbrush_t xb;
    default_xbrush(&xb);

    xrect_t xr;

	if (bd.edge)
	{
		(*if_widget.pf_widget_get_client_rect)(wt, &xr);
        xsprintf(xp.size, _T("%d"), bd.edge);
        (*if_context.pf_gdi_draw_rect)(dc, &xp, NULL, &xr);
        xsprintf(xp.size, _T("%d"), 1);
	}

	if (bd.title)
	{
		(*if_widget.pf_widget_get_client_rect)(wt, &xr);
        xscpy(xb.color, GDI_ATTR_RGB_BLUE);
        xr.h = bd.title;
        (*if_context.pf_gdi_draw_rect)(dc, &xp, &xb, &xr);
	}

	if (bd.menu)
	{
		(*if_widget.pf_widget_get_client_rect)(wt, &xr);
        xscpy(xb.color, GDI_ATTR_RGB_RED);
        xr.y = bd.title;
        xr.h = bd.menu;
        (*if_context.pf_gdi_draw_rect)(dc, &xp, &xb, &xr);
	}

	if (bd.hscroll)
	{
		(*if_widget.pf_widget_get_client_rect)(wt, &xr);
        xscpy(xb.color, GDI_ATTR_RGB_GRAY);
        xr.y = xr.y + xr.h - bd.hscroll;
        xr.h = bd.hscroll;
        (*if_context.pf_gdi_draw_rect)(dc, &xp, &xb, &xr);
	}

	if (bd.vscroll)
	{
		(*if_widget.pf_widget_get_client_rect)(wt, &xr);
        xscpy(xb.color, GDI_ATTR_RGB_GRAY);
        xr.x = xr.w - bd.vscroll;
        xr.w = bd.vscroll;
        xr.y = bd.title + bd.menu;
        xr.h = xr.h - (bd.title + bd.menu + bd.hscroll);
        (*if_context.pf_gdi_draw_rect)(dc, &xp, &xb, &xr);
	}
}

void main_on_nc_calcsize(res_win_t wt, border_t* pbd)
{
    pbd->edge = 5;
    pbd->title = 30;
    pbd->hscroll = 10;
    pbd->vscroll = 10;

    printf("main on_nc_calcsize edge: %d, title: %d, hscroll: %d, vscroll: %d\n", 
        pbd->edge, pbd->title, pbd->hscroll, pbd->vscroll);
}

int main_on_nc_hittest(res_win_t wt, const xpoint_t* pxp)
{
    printf("main on_nc_hinttest x: %d, y: %d\n", 
        pxp->x, pxp->y);

	int edge, title, vscr, hscr, menu;
	border_t bd = { 0 };
	dword_t ws;
	xrect_t xr, rt;
	xpoint_t pt;

	ws = (*if_widget.pf_widget_get_style)(wt);

	main_on_nc_calcsize(wt, &bd);

	edge = bd.edge;
	title = bd.title;
	hscr = bd.hscroll;
	vscr = bd.vscroll;
	menu = bd.menu;

	if (!edge && !title && !vscr && !hscr && !menu)
		return HINT_CLIENT;

	(*if_widget.pf_widget_get_window_rect)(wt, &xr);

	pt.x = pxp->x;
	pt.y = pxp->y;

	if (menu)
	{
		rt.x = xr.x + edge;
		rt.w = xr.w - 2 * edge;
		rt.y = xr.y + edge + title;
		rt.h = menu;

		if (pt_in_rect(pxp, &rt))
			return HINT_MENUBAR;
	}

	if (hscr)
	{
		rt.x = xr.x + edge;
		rt.w = hscr;
		rt.y = xr.y + xr.h - edge - hscr;
		rt.h = hscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_LINELEFT;

		rt.x = xr.x + edge + hscr;
		rt.w = xr.w - 2 * edge - vscr - 2 * hscr;
		rt.y = xr.y + xr.h - edge - hscr;
		rt.h = hscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_HSCROLL;

		rt.x = xr.x + xr.w - edge - vscr - hscr;
		rt.w = hscr;
		rt.y = xr.y + xr.h - edge - hscr;
		rt.h = hscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_LINERIGHT;
	}

	if (vscr)
	{
		rt.x = xr.x + xr.w - edge - vscr;
		rt.w = vscr;
		rt.y = xr.y + edge + title + menu;
		rt.h = vscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_PAGEUP;

		rt.x = xr.x + xr.w - edge - vscr;
		rt.w = vscr;
		rt.y = xr.y + edge + title + menu + vscr;
		rt.h = vscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_LINEUP;

		rt.x = xr.x + xr.w - edge - vscr;
		rt.w = vscr;
		rt.y = xr.y + edge + title + menu + 2 * vscr;
		rt.h = xr.h - 2 * edge - title - menu - 4 * vscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_VSCROLL;

		rt.x = xr.x + xr.w - edge - vscr;
		rt.w = vscr;
		rt.y = xr.y + xr.h - edge - 2 * vscr;
		rt.h = vscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_LINEDOWN;

		rt.x = xr.x + xr.w - edge - vscr;
		rt.w = vscr;
		rt.y = xr.y + xr.h - edge - vscr;
		rt.h = vscr;

		if (pt_in_rect(pxp, &rt))
			return HINT_PAGEDOWN;
	}

	if (title)
	{
		rt.x = xr.x + edge;
		rt.w = title;
		rt.y = xr.y + edge;
		rt.h = title;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_ICON;
		}

		rt.x = xr.x + xr.w - edge - (title / 2 * 3);
		rt.w = title / 2;
		rt.y = xr.y + edge;
		rt.h = title;
		if (pt_in_rect(pxp, &rt))
		{
			if (ws & WD_STYLE_SIZEBOX)
				return HINT_MINIMIZE;
			else
				return HINT_TITLE;
		}

		rt.x = xr.x + xr.w - edge - title;
		rt.w = title / 2;
		rt.y = xr.y + edge;
		rt.h = title;
		if (pt_in_rect(pxp, &rt))
		{
			if (ws & WD_STYLE_SIZEBOX)
			{
				if ((*if_widget.pf_widget_is_maximized)(wt))
					return HINT_RESTORE;
				else
					return HINT_MAXIMIZE;
			}
			else
				return HINT_TITLE;
		}

		rt.x = xr.x + xr.w - edge - (title / 2);
		rt.w = title / 2;
		rt.y = xr.y + edge;
		rt.h = title;
		if (pt_in_rect(pxp, &rt))
		{
			if (ws & WD_STYLE_CLOSEBOX)
				return HINT_CLOSE;
			else
				return HINT_TITLE;
		}

		rt.x = xr.x + edge;
		rt.w = xr.w - 2 * edge;
		rt.y = xr.y + edge;
		rt.h = title;

		if (pt_in_rect(pxp, &rt))
			return HINT_TITLE;
	}

	if (edge)
	{
		rt.x = xr.x;
		rt.w = edge;
		rt.y = xr.y;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_TOPLEFT;
		}

		rt.x = xr.x + edge;
		rt.w = xr.w - 2 * edge;
		rt.y = xr.y;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_TOP;
		}

		rt.x = xr.x + xr.w - edge;
		rt.w = edge;
		rt.y = xr.y;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_TOPRIGHT;
		}

		rt.x = xr.x;
		rt.w = edge;
		rt.y = xr.y + edge;
		rt.h = xr.h - 2 * edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_LEFT;
		}

		rt.x = xr.x + xr.w - edge;
		rt.w = edge;
		rt.y = xr.y + edge;
		rt.h = xr.h - 2 * edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_RIGHT;
		}

		rt.x = xr.x + xr.w - edge;
		rt.w = edge;
		rt.y = xr.y + xr.h - edge;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_RIGHTBOTTOM;
		}

		rt.x = xr.x + edge;
		rt.w = xr.w - 2 * edge;
		rt.y = xr.y + xr.h - edge;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_BOTTOM;
		}

		rt.x = xr.x;
		rt.w = edge;
		rt.y = xr.y + xr.h - edge;
		rt.h = edge;
		if (pt_in_rect(pxp, &rt))
		{
			return HINT_LEFTBOTTOM;
		}
	}

	rt.x = xr.x + edge;
	rt.w = xr.w - 2 * edge - vscr;
	rt.y = xr.y + edge + title + menu;
	rt.h = xr.h - 2 * edge - title - menu - hscr;
	if (pt_in_rect(pxp, &rt))
	{
		return HINT_CLIENT;
	}

	return HINT_NOWHERE;
}

/********************************************************************************/

void init_instance()
{
    xdu_impl_context(&if_context);

    xdu_impl_context_graphic(&if_context);

    xdu_impl_context_bitmap(&if_context);

    (*if_context.pf_context_startup)();

    xdu_impl_widget(&if_widget);

    (*if_widget.pf_widget_startup)(0);

    accel_t acs[3] = {
        {KEY_CONTROL, 'a', 10},
        {KEY_ALT, 's', 20},
        {KEY_SHIFT, '\t', 30}
    };

    res_acl_t acl = (*if_widget.pf_create_accel_table)(acs, 3);

    if_event_t ev = {0};

    ev.pf_on_create = main_on_create;
	ev.pf_on_close = main_on_close;
	ev.pf_on_destroy = main_on_destroy;
	ev.pf_on_lbutton_down = main_on_lbutton_down;
	ev.pf_on_lbutton_up = main_on_lbutton_up;
	ev.pf_on_lbutton_dbclick = main_on_lbutton_dbclick;
	ev.pf_on_rbutton_down = main_on_rbutton_down;
	ev.pf_on_rbutton_up = main_on_rbutton_up;
	ev.pf_on_mouse_move = main_on_mouse_move;
	ev.pf_on_mouse_hover = main_on_mouse_hover;
	ev.pf_on_mouse_leave = main_on_mouse_leave;
	ev.pf_on_wheel = main_on_whell;
	ev.pf_on_scroll = main_on_scroll;
	ev.pf_on_keydown = main_on_keydown;
	ev.pf_on_keyup = main_on_keyup;
	ev.pf_on_char = main_on_char;
	ev.pf_on_size = main_on_size;
	ev.pf_on_move = main_on_move;
	ev.pf_on_show = main_on_show;
	ev.pf_on_activate = main_on_activate;
	ev.pf_on_set_focus = main_on_set_focus;
	ev.pf_on_kill_focus = main_on_kill_focus;
	ev.pf_on_paint = main_on_paint;
	ev.pf_on_notice = main_on_notice;
	ev.pf_on_menu_command = main_on_menu_command;
	ev.pf_on_parent_command = main_on_parent_command;
	ev.pf_on_child_command = main_on_child_command;
	ev.pf_on_self_command = main_on_self_command;
    ev.pf_on_scroll = main_on_scroll;

    //ev.pf_on_nc_calcsize = main_on_nc_calcsize;
    //ev.pf_on_nc_hittest = main_on_nc_hittest;
    //ev.pf_on_nc_paint = main_on_nc_paint;

    xrect_t xr = {0};
    xr.x = 100;
	xr.y = 100;
	xr.w = 600;
	xr.h = 300;

	g_main = (*if_widget.pf_widget_create)(_T("frame1"), WD_STYLE_FRAME, &xr, NULL, &ev);
	(*if_widget.pf_widget_set_accel)(g_main, acl);

	(*if_widget.pf_widget_update)(g_main);
    (*if_widget.pf_widget_show)(g_main, WS_SHOW_NORMAL);

    scroll_t scr = {0};
    scr.max = 1000;
    scr.min = 10;

    (*if_widget.pf_widget_set_scroll_info)(g_main, 0, &scr);

    (*if_widget.pf_widget_active)(g_main);
}

void uninit_instance()
{
    (*if_widget.pf_widget_cleanup)();

    (*if_context.pf_context_cleanup)();
}

void test_win()
{
	init_instance();

	(*if_widget.pf_widget_do_normal)(g_main);

	uninit_instance();
}

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

int main(int argc, const char * argv[]) {

    xdk_process_init(XDK_APARTMENT_PROCESS);

	//test_blut();

	test_win();

    xdk_process_uninit();

    return 0;
}
