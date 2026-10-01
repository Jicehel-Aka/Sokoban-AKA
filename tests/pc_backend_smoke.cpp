// Smoke test of the PC backend: the real gb_graphics draws into the framebuffer, the SDL layer shows it.
// SPDX-License-Identifier: MIT
#include "gamebuino.h"

extern "C" void app_main(void)
{
    gb_core core;
    gb_graphics gfx;
    core.init();
    gfx.clear(gfx.makeColor(20, 40, 90));
    gfx.setColor(gfx.makeColor(255, 0, 0));   gfx.fillRect(10, 10, 60, 40);
    gfx.setColor(gfx.makeColor(0, 255, 0));   gfx.fillRect(80, 10, 60, 40);
    gfx.setColor(gfx.makeColor(0, 0, 255));   gfx.fillRect(150, 10, 60, 40);
    gfx.setColor(gfx.makeColor(255, 255, 255));
    gfx.drawCircle(60, 120, 30);
    gfx.move_cursor(10, 200);
    gfx.print_str("gb_graphics on PC");
    for (int frame = 0;; ++frame) {
        core.pool();
        gfx.setColor(gfx.makeColor(255, 255, 0));
        gfx.fillRect(10 + (frame % 280), 170, 8, 8);
        gfx.update();
    }
}
