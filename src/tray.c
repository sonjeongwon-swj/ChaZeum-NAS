#include "serverlife.h"
typedef struct { U32 size; H hwnd; U32 id,flags,event; H icon; W tip[128]; U32 state,mask; W info[256]; U32 version; W name[64]; U32 flags2; Guid guid; H balloon; } NotifyIcon;
_Static_assert(sizeof(NotifyIcon)==976&&offsetof(NotifyIcon,tip)==40,"Windows notification icon ABI");
extern I32 Shell_NotifyIconW(U32,NotifyIcon*),SetForegroundWindow(H);

extern U32 RegisterWindowMessageW(T);
static U32 taskbar;static int hidden;
static int iconop(H h,U32 op){
 NotifyIcon n={0};n.size=sizeof(n);n.hwnd=h;n.id=1;
 if(!op){n.flags=1|2|4;n.event=CZ_TRAY_EVENT;n.icon=brandicon(GetModuleHandleW(0),1);
  if(!n.icon)return 0;cp(n.tip,CNT(n.tip),L"차즘 NAS · 창 다시 열기");}
 return Shell_NotifyIconW(op,&n);
}
void tray_init(void){taskbar=RegisterWindowMessageW(L"TaskbarCreated");}
void tray_done(H h){if(hidden){iconop(h,2);hidden=0;}}
int tray_hide(H h){if(!taskbar||hidden||!iconop(h,0))return 0;hidden=1;ShowWindow(h,0);return 1;}
static void restoreui(H h){ShowWindow(h,9);SetForegroundWindow(h);tray_done(h);}
void tray_event(H h,U32 m,N l){
 if(!hidden)return;
 /* Restore the window if Explorer restarts or an accessibility user activates the icon. */
 if(m==taskbar||(m==CZ_TRAY_EVENT&&(l==0x202||l==0x203||l==0x205||l==0x400||l==0x401)))restoreui(h);
}
