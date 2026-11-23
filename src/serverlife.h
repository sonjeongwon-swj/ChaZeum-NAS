#ifndef CZ_SERVER_LIFE
#define CZ_SERVER_LIFE
#include "base.h"
/* Both settings control the native Windows Server service, not the lifetime of the UI. */
int server_service(int automatic, W *out, int cap);
int server_snapshot(W *out, int cap);
int count_owned_shares(U32 *count);
int tray_hide(H window);
void tray_event(H window, U32 msg, N arg);
void tray_init(void);
void tray_done(H window);
#define CZ_TRAY_EVENT 0x8051
#endif
