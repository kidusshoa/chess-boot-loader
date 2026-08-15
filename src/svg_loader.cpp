#include "svg_loader.h"

#include <iostream>
using namespace std;

#include <librsvg/rsvg.h>

#include <cairo/cairo.h>

#include <cstring>

SDL_Surface* cairo_surface_to_sdl(cairo_surface_t* cairo_surface, int width, int height) {
    cairo_surface_flush(cairo_surface);

    SDL_Surface* sdl_surface = SDL_CreateRGBSurfaceWithFormat(
        0,
        width,
        height,
        32,
        SDL_PIXELFORMAT_ABGR8888
    );
    if (!sdl_surface) {
        return nullptr;
    }

    const unsigned char* src = cairo_image_surface_get_data(cairo_surface);
    const int src_stride = cairo_image_surface_get_stride(cairo_surface);
    unsigned char* dst = static_cast<unsigned char*>(sdl_surface->pixels);
    const int dst_stride = sdl_surface->pitch;

    for (int y = 0; y < height; ++y) {
        memcpy(dst + y * dst_stride, src + y * src_stride, static_cast<size_t>(width) * 4);
    }

    SDL_Surface* converted = SDL_ConvertSurfaceFormat(sdl_surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(sdl_surface);
    return converted;
}



SDL_Surface* load_svg_surface(const char* path, int width, int height) {
    GError* error = nullptr;
    RsvgHandle* handle = rsvg_handle_new_from_file(path, &error);
    if (!handle) {
        if (error) {
            cerr << "rsvg load failed for " << path << ": " << error->message << "\n";
            g_error_free(error);
        }
        return nullptr;
    }

    cairo_surface_t* cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS) {
        g_object_unref(handle);
        return nullptr;
    }

    cairo_t* cr = cairo_create(cairo_surface);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    const RsvgRectangle viewport = {
        0.0,
        0.0,
        static_cast<double>(width),
        static_cast<double>(height),
    };

    if (!rsvg_handle_render_document(handle, cr, &viewport, &error)) {
        cerr << "rsvg render failed for " << path << ": "
             << (error ? error->message : "unknown error") << "\n";
        if (error) {
            g_error_free(error);
        }
        cairo_destroy(cr);
        cairo_surface_destroy(cairo_surface);
        g_object_unref(handle);
        return nullptr;
    }

    cairo_destroy(cr);
    g_object_unref(handle);

    SDL_Surface* sdl_surface = cairo_surface_to_sdl(cairo_surface, width, height);
    cairo_surface_destroy(cairo_surface);
    return sdl_surface;
}
