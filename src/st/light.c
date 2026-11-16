/*
 * ST.EXE: the lighting environments: filling the table from its defaults,
 * rebuilding the light matrices, blending default parts into the table,
 * selecting an environment from row/column flags and blending the current
 * environment toward it (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <st/gfx.h>
#include <st/data.h>
#include <st/light.h>

/* Fills the environments from the defaults. */
ADDRESS(0x80032058, 0x114)
void lighting_load_defaults(void)
{
    StLightInit *init = DAT_8007f78c;
    StLight *light = lighting.light;
    int i = 64;

    while (i--) {
        light->color = init->color;
        light->light[0] = init->unk0;
        light->back = init->back;
        light->unk68 = init->unk2C;
        light->unk6A = init->unk2E;
        init++;
        light++;
    }
    lighting.unk2 = 1;
}

/* Derives every environment's light matrices 0..3 from matrix 0. */
ADDRESS(0x8003216c, 0xa4)
void lighting_build_matrices(void)
{
    StLight *light;
    int i;

    if (lighting.unk2) {
        light = lighting.light;
        i = 64;
        while (i--) {
            func_80049aac(light, &light->light[0], 0);
            func_80049aac(light, &light->light[1], 1);
            func_80049aac(light, &light->light[2], 2);
            func_80049aac(light, &light->light[3], 3);
            light++;
        }
        lighting.unk2 = 0;
    }
}

ADDRESS(0x80032210, 0x34)
void lighting_blend_matrix(StLightInit *init, StLight *light, short t)
{
    func_8004a888(&light->light[0], &init->unk0, &light->light[0], t);
}

ADDRESS(0x80032244, 0x38)
void lighting_blend_color(StLightInit *init, StLight *light, short t)
{
    func_8004a888(&light->color, &init->color, &light->color, t);
}

ADDRESS(0x8003227c, 0x7c)
void lighting_blend_back(StLightInit *init, StLight *light, short t)
{
    light->back.r = func_8004a830(light->back.r, init->back.r, t);
    light->back.g = func_8004a830(light->back.g, init->back.g, t);
    light->back.b = func_8004a830(light->back.b, init->back.b, t);
}

/* Blends unk68, keeping unk6A at least 4096 beyond it. */
ADDRESS(0x800322f8, 0x60)
void lighting_blend_near(StLightInit *init, StLight *light, short t)
{
    light->unk68 = func_8004a830(light->unk68, init->unk2C, t);
    if (light->unk68 > light->unk6A - 4096)
        light->unk6A = light->unk68 + 4096;
}

/* Blends unk6A, keeping unk68 at least 4096 below it. */
ADDRESS(0x80032358, 0x60)
void lighting_blend_far(StLightInit *init, StLight *light, short t)
{
    light->unk6A = func_8004a830(light->unk6A, init->unk2E, t);
    if (light->unk6A < light->unk68 + 4096)
        light->unk68 = light->unk6A - 4096;
}

/*
 * Blends the parts of `init` that `flags` selects into the first 48
 * environments: 1 matrix 0, 2 the colour matrix, 4 the back colour,
 * 8 unk68, 0x10 unk6A.
 */
ADDRESS(0x800323b8, 0x178)
void lighting_blend(int flags, StLightInit *init, short t)
{
    StLight *light;
    int i;

    if (flags & 1) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_matrix(init, light, t);
            light++;
        }
        lighting.unk2 = 1;
    }
    if (flags & 2) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_color(init, light, t);
            light++;
        }
    }
    if (flags & 4) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_back(init, light, t);
            light++;
        }
    }
    if (flags & 8) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_near(init, light, t);
            light++;
        }
    }
    if (flags & 0x10) {
        light = lighting.light;
        for (i = 0; i < 48; i++) {
            lighting_blend_far(init, light, t);
            light++;
        }
    }
}

/* Blends a back colour (no part when any component is 255) and the two
 * halfwords (none when 0xffff) into the environments. */
ADDRESS(0x80032530, 0x94)
void lighting_blend_back_ranges(r, g, b, unk68, unk6A, t)
u8 r;
u8 g;
u8 b;
int unk68;
int unk6A;
short t;
{
    StLightInit init;
    int flags = 0;

    if (r != 255 && g != 255 && b != 255) {
        flags = 4;
        init.back.r = r;
        init.back.g = g;
        init.back.b = b;
    }
    if (unk68 != 0xffff) {
        init.unk2C = unk68;
        flags |= 8;
    }
    if (unk6A != 0xffff) {
        init.unk2E = unk6A;
        flags |= 0x10;
    }
    lighting_blend(flags, &init, t);
}

/* Blends a colour matrix and a light matrix (none when any element is
 * 0x7fff) into the environments. */
ADDRESS(0x800325c4, 0x1b0)
void lighting_blend_matrices(StLightMatrix color, StLightMatrix matrix, short t)
{
    StLightInit init;
    int flags = 0;

    if (color.m[0][0] != 0x7fff && color.m[1][0] != 0x7fff && color.m[2][0] != 0x7fff &&
        color.m[0][1] != 0x7fff && color.m[1][1] != 0x7fff && color.m[2][1] != 0x7fff &&
        color.m[0][2] != 0x7fff && color.m[1][2] != 0x7fff && color.m[2][2] != 0x7fff) {
        flags = 2;
        init.color.m[0][0] = color.m[0][0];
        init.color.m[1][0] = color.m[1][0];
        init.color.m[2][0] = color.m[2][0];
        init.color.m[0][1] = color.m[0][1];
        init.color.m[1][1] = color.m[1][1];
        init.color.m[2][1] = color.m[2][1];
        init.color.m[0][2] = color.m[0][2];
        init.color.m[1][2] = color.m[1][2];
        init.color.m[2][2] = color.m[2][2];
    }
    if (matrix.m[0][0] != 0x7fff && matrix.m[0][1] != 0x7fff && matrix.m[0][2] != 0x7fff &&
        matrix.m[1][0] != 0x7fff && matrix.m[1][1] != 0x7fff && matrix.m[1][2] != 0x7fff &&
        matrix.m[2][0] != 0x7fff && matrix.m[2][1] != 0x7fff && matrix.m[2][2] != 0x7fff) {
        flags |= 1;
        init.unk0.m[0][0] = matrix.m[0][0];
        init.unk0.m[0][1] = matrix.m[0][1];
        init.unk0.m[0][2] = matrix.m[0][2];
        init.unk0.m[1][0] = matrix.m[1][0];
        init.unk0.m[1][1] = matrix.m[1][1];
        init.unk0.m[1][2] = matrix.m[1][2];
        init.unk0.m[2][0] = matrix.m[2][0];
        init.unk0.m[2][1] = matrix.m[2][1];
        init.unk0.m[2][2] = matrix.m[2][2];
    }
    lighting_blend(flags, &init, t);
}

/* The grid environment of the lowest row flag (0x10..0x80) and the lowest
 * column flag (1..4). */
ADDRESS(0x80032774, 0x8c)
int lighting_grid_index(flags)
u8 flags;
{
    int row = 0;
    int column = 0;

    if (flags & 0x10)
        row = 0;
    else if (flags & 0x20)
        row = 1;
    else if (flags & 0x40)
        row = 2;
    else if (flags & 0x80)
        row = 3;
    if (flags & 1)
        column = 0;
    else if (flags & 2)
        column = 1;
    else if (flags & 4)
        column = 2;
    return row * 3 + column;
}

/*
 * Selects the environment of the row and column flags. Without `blend` it
 * only records the selection; with it the current environment becomes the
 * blend start and lighting_update blends toward the selection.
 */
ADDRESS(0x80032800, 0x86c)
void lighting_select(rows, columns, blend)
u8 rows;
u8 columns;
int blend;
{
    int row;
    int column;
    u8 bits;
    u8 flags;

    row = 0;
    for (bits = rows; !(bits & 0x10); row++)
        bits >>= 1;
    column = 0;
    for (bits = columns; !(bits & 1); column++)
        bits >>= 1;
    world_state.unk1EE14 = rows | columns;
    if (!blend) {
        world_state.render_mode = rows | columns;
        world_state.unk1EDFD = row * 3 + column;
        return;
    }
    if (!(world_state.unk1EDFE & 0x80)) {
        if (world_state.render_mode & 2) {
            if (world_state.render_mode & 0x10)
                *world_state.light = lighting.light[12];
            else if (world_state.render_mode & 0x20)
                *world_state.light = lighting.light[13];
            else if (world_state.render_mode & 0x40)
                *world_state.light = lighting.light[15];
            else if (world_state.render_mode & 0x80)
                *world_state.light = lighting.light[17];
        } else if (world_state.render_mode & 4) {
            if (world_state.render_mode & 0x20)
                *world_state.light = lighting.light[14];
            else if (world_state.render_mode & 0x40)
                *world_state.light = lighting.light[16];
            else if (world_state.render_mode & 0x80)
                *world_state.light = lighting.light[18];
        } else {
            *world_state.light = lighting.light[lighting_grid_index(world_state.render_mode)];
        }
    }
    world_state.unk1EDFD = row * 3 + column;
    world_state.unk1EE10 = &lighting.light[world_state.unk1EDFD];
    flags = world_state.unk1EE14;
    if (flags & 2) {
        if (flags & 0x10)
            world_state.unk1EE10 = &lighting.light[12];
        else if (flags & 0x20)
            world_state.unk1EE10 = &lighting.light[13];
        else if (flags & 0x40)
            world_state.unk1EE10 = &lighting.light[15];
        else if (flags & 0x80)
            world_state.unk1EE10 = &lighting.light[17];
    } else if (flags & 4) {
        if (flags & 0x20)
            world_state.unk1EE10 = &lighting.light[14];
        else if (flags & 0x40)
            world_state.unk1EE10 = &lighting.light[16];
        else if (flags & 0x80)
            world_state.unk1EE10 = &lighting.light[18];
    }
    *world_state.unk1EE0C = *world_state.light;
    world_state.render_mode = 0;
    world_state.unk1EDFE = 0x80;
}

/* Points the current and blend-start environments at their table slots
 * and selects the current area's environment without blending. */
ADDRESS(0x8003306c, 0x84)
void lighting_start(void)
{
    u8 mode;

    world_state.light = &lighting.light[43];
    world_state.unk1EE0C = &lighting.light[44];
    lighting.unk4 = 0;
    lighting.unk6 = 0;
    lighting.unk3 = 0;
    lighting.unk5 = 0;
    lighting.unk4 = 0;
    lighting.unk3 = 0;
    lighting.unk7 = 0;
    mode = lighting.area_mode[area_state.area];
    lighting_select(mode & 0xf0, mode & 0xf, 0);
}

ADDRESS(0x800330f0, 0x20)
void lighting_clear_requests(void)
{
    lighting.unk9 = 0;
    lighting.unk4 = 0;
    lighting.unk3 = 0;
    lighting.unk6 = 0;
    lighting.unk5 = 0;
}

/* An environment field blended from unk1EE0C to unk1EE10 by t / 64 with
 * weight w for the start (unsigned for the colours, signed for the
 * ranges). */
#define LIGHT_BLEND(field, w) \
    ((world_state.unk1EE0C->field * (w) + world_state.unk1EE10->field * t) >> 6)

/*
 * Per frame: picks the row and column flags for the current area and
 * state, starts a blend when they change and advances the blend of the
 * current environment over 64 steps.
 */
ADDRESS(0x80033110, 0x8b4)
void lighting_update(void)
{
    u8 mode;
    u8 columns;
    int rows;
    u8 t;
    u32 s;

    mode = lighting.area_mode[area_state.area];
    columns = mode & 0xf;
    rows = mode & 0xf0;
    if (area_state.area == 25 || area_state.area == 26 || area_state.area == 14 ||
        area_state.area == 23) {
        lighting.unk9 = 0;
        lighting.unk4 &= ~0x20;
    }
    if (viewer.unk1C7 < 0) {
        lighting.unk3 |= 1;
        lighting.unk4 |= 4;
    } else if (viewer.unk1C7 > 0) {
        lighting.unk3 |= 1;
        lighting.unk4 |= 0x10;
    }
    if (lighting.unkA == 1)
        return;
    if (lighting.unkA == 2) {
        lighting.unkA = 0;
        return;
    }
    if (area_state.area != lighting.unk7) {
        lighting.unk8 = 1;
        lighting.unk7 = area_state.area;
        world_state.unk1EDFE = 0;
    }
    if (lighting.unk8 == 2 && viewer.unk218 == 6) {
        lighting.unk3 = 0;
        lighting.unk4 = 0;
    }
    if (lighting.unk9) {
        columns = 1;
        lighting.unk9 = 0;
    } else if (lighting.area_mode[area_state.area] == 17) {
        columns = 1;
    } else if ((columns | lighting.unk3) & 4) {
        columns = 4;
    } else if ((columns | lighting.unk3) & 2) {
        columns = 2;
    } else {
        columns = 1;
    }
    if (lighting.unk4) {
        if (lighting.unk4 & 0xe0)
            rows = 16;
        else if (lighting.unk4 & 0xf)
            rows = 64;
        else if (lighting.area_mode[area_state.area] & 0x10)
            rows = 16;
        else
            rows = 32;
    }
    if (columns != lighting.unk5 || rows != lighting.unk6) {
        if (lighting.unk8 == 2) {
            lighting_select(rows, columns, 0);
            lighting.unk5 = columns;
            lighting.unk6 = rows;
        } else if (lighting.unk8 == 0) {
            lighting_select(rows, columns, 1);
            lighting.unk5 = columns;
            lighting.unk6 = rows;
        }
    }
    if (lighting.unk8 == 1 && (short)area_state.unk2 >= 7)
        lighting.unk8 = 2;
    else if (lighting.unk8 == 2)
        lighting.unk8 = 0;
    lighting.unk4 = 0;
    lighting.unk3 = 0;
    if (world_state.unk1EDFE & 0x80) {
        t = world_state.unk1EDFE & 0x7f;
        if (t >= 64) {
            world_state.unk1EDFE = 0;
            world_state.render_mode = world_state.unk1EE14;
            *world_state.light = lighting.light[world_state.unk1EDFD];
        }
        world_state.light->unk68 = LIGHT_BLEND(unk68, 64 - t);
        world_state.light->unk6A = LIGHT_BLEND(unk6A, 64 - t);
        s = 64 - t;
        world_state.light->color.m[0][0] = LIGHT_BLEND(color.m[0][0], s);
        world_state.light->color.m[0][1] = LIGHT_BLEND(color.m[0][1], s);
        world_state.light->color.m[0][2] = LIGHT_BLEND(color.m[0][2], s);
        world_state.light->color.m[1][0] = LIGHT_BLEND(color.m[1][0], s);
        world_state.light->color.m[1][1] = LIGHT_BLEND(color.m[1][1], s);
        world_state.light->color.m[1][2] = LIGHT_BLEND(color.m[1][2], s);
        world_state.light->color.m[2][0] = LIGHT_BLEND(color.m[2][0], s);
        world_state.light->color.m[2][1] = LIGHT_BLEND(color.m[2][1], s);
        world_state.light->color.m[2][2] = LIGHT_BLEND(color.m[2][2], s);
        world_state.light->back.r = LIGHT_BLEND(back.r, s);
        world_state.light->back.g = LIGHT_BLEND(back.g, s);
        world_state.light->back.b = LIGHT_BLEND(back.b, s);
        world_state.unk1EDFE++;
    } else {
        *world_state.light = lighting.light[world_state.unk1EDFD];
    }
    world_state.unk1EE00 = world_state.light->unk68 - 1024;
    world_state.unk1EE04 = 4096;
}
