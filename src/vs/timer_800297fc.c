/*
 * VS.EXE: the round clock (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <vs/data.h>

/*
 * Draws the seconds left of `time` (frames, rounded up) as two digits
 * cross-fading from the previous second's digits over the first 16 frames
 * of each second, reddening over the last 14 seconds.
 */
/* Not exact: retail keeps seconds/frame/warning in t1/t2/t3;
 * this build cycles them through t3/t1/t2. */
ADDRESS(0x800297fc, 0x314)
void func_800297fc(int time)
{
    int secs = time + 59;
    int frame = secs % 60;
    int warn;
    int i;
    int digit;
    StPacketBuffer *packet;
    POLY_FT4 *p;

    secs /= 60;
    warn = (14 - secs) * 2;
    if (warn < 0)
        warn = 0;
    if (warn > 8)
        warn = 8;
    for (i = 0; i < 4; i++) {
        packet = DAT_801624d0.packet;
        p = (POLY_FT4 *)packet->cur;
        packet->cur = (u_long *)(p + 1);
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXY4(p, i % 2 * 32 + 128, 16, i % 2 * 32 + 160, 16,
               i % 2 * 32 + 128, 48, i % 2 * 32 + 160, 48);
        switch (i) {
        case 0:
            digit = secs / 10;
            break;
        case 1:
            digit = secs % 10;
            break;
        case 2:
            digit = (secs - 1) / 10;
            break;
        case 3:
            digit = (secs - 1) % 10;
            break;
        }
        if (digit < 5) {
            setUV4(p, digit * 32, 0, digit * 32 + 32, 0,
                   digit * 32, 32, digit * 32 + 32, 32);
        } else {
            digit -= 5;
            setUV4(p, digit * 32, 32, digit * 32 + 32, 32,
                   digit * 32, 64, digit * 32 + 32, 64);
        }
        p->clut = 32290;
        p->tpage = 61;
        if (i < 2) {
            if (frame >= 16) {
                setRGB0(p, warn * 16 + 127, -128 - warn * 16, -128 - warn * 16);
            } else {
                setRGB0(p, frame * (warn + 8), frame * (8 - warn), frame * (8 - warn));
            }
        } else if (frame >= 16) {
            setRGB0(p, 0, 0, 0);
        } else {
            setRGB0(p, warn * 16 + 127 - frame * (warn + 8),
                    -128 - warn * 16 - frame * (8 - warn),
                    -128 - warn * 16 - frame * (8 - warn));
        }
        addPrim(&DAT_801624d0.ot_cur[3], p);
    }
}
