#pragma once

// Framebuffer module: responsible for rendering the framebuffer to screen.
void render_init_framebuffer();
void render_framebuffer();

// General rendering: responsible for generating lists and rendering the lists.
void render_init_objects();
void render_objects();

// Sprite rendering: specific data for sprite rendering.
void render_init_sprite();

// Updates frame-constant shader params (i.e. the projection matrix)
void render_refresh_sprite();

// Binds the vertex arrays and shader data for sprites
void render_bind_sprite();
