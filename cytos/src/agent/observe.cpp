#include <cairo/cairo.h>

#include "../entity/control.hpp"
#include "../physics/engine.hpp"
#include "agent.hpp"

static thread_local std::unique_ptr<cairo_surface_t,
                                    decltype(&cairo_surface_destroy)>
    surface_ptr(nullptr, &cairo_surface_destroy);

struct RGBColor {
    float r, g, b;
    RGBColor(){};
    RGBColor(int r, int g, int b) : r(r / 255.f), g(g / 255.f), b(b / 255.f){};
};

static const RGBColor COLORS[12] = {
    RGBColor(0, 255, 148),  RGBColor(132, 255, 61),  RGBColor(255, 152, 67),
    RGBColor(0, 240, 234),  RGBColor(255, 0, 98),    RGBColor(0, 169, 255),
    RGBColor(255, 226, 0),  RGBColor(148, 0, 255),   RGBColor(196, 4, 78),
    RGBColor(192, 255, 58), RGBColor(255, 106, 228), RGBColor(255, 142, 0),
};

static inline void cairo_set_source_color(cairo_t* ctx, RGBColor color) {
    cairo_set_source_rgb(ctx, color.r, color.g, color.b);
};

static inline void cairo_circle(cairo_t* ctx, double x, double y, double r) {
    cairo_new_path(ctx);
    cairo_arc(ctx, x, y, r, 0, 2 * M_PI);
    cairo_close_path(ctx);
}

string_view Agent::observe() {
    if (!control || !dual || !dual->control) return string_view(nullptr, 0);

    if (surface_ptr == nullptr) {
        surface_ptr.reset(cairo_image_surface_create(
            cairo_format_t::CAIRO_FORMAT_RGB24, DIM, DIM));
    }

    uint16_t t0 = control->id;
    uint16_t t1 = dual->control->id;

    auto surface = surface_ptr.get();
    // Get a ctx from the surface
    cairo_t* ctx = cairo_create(surface);

    auto vl = std::max(control->viewport.hw, control->viewport.hh);
    auto& v = control->viewport;

    double scale = DIM / (vl * 2);
    double t_x = -((v.x - vl) * scale);
    double t_y = -((v.y - vl) * scale);

    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, scale, 0.0, 0.0, scale, t_x, t_y);
    cairo_transform(ctx, &matrix);

    auto map = engine->getMap();

    cairo_set_antialias(ctx, CAIRO_ANTIALIAS_GRAY);
    // Draw background
    cairo_set_source_rgb(ctx, 0, 0, 0);
    cairo_paint(ctx);
    // Draw map
    cairo_set_source_color(ctx, RGBColor(17, 17, 17));
    cairo_rectangle(ctx, map.x - map.hw, map.y - map.hh, map.hw * 2,
                    map.hh * 2);
    cairo_fill(ctx);

    auto aabb = v.toAABB();

    cairo_set_source_color(ctx, RGBColor(255, 255, 255));
    // Query pellets, no need to sort
    engine->queryGridPL(aabb, [&](Cell* cell) {
        cairo_circle(ctx, cell->x, cell->y, cell->r);
        cairo_fill(ctx);
    });

    // Query ejected + virus, no need to sort(?)
    engine->queryGridEV(aabb, [&](Cell* cell) {
        cairo_set_source_color(ctx, cell->type & EJECT_BIT
                                        ? RGBColor(200, 200, 200)
                                        : RGBColor(0, 255, 0));
        cairo_circle(ctx, cell->x, cell->y, cell->r);
        cairo_fill(ctx);
    });

    vector<Cell*> cells;

    engine->queryTree(aabb, [&](Cell* cell) {
        // Don't draw EXP/CYT cells for now
        if (cell->type == EXP_TYPE || cell->type == CYT_TYPE) return;
        cells.push_back(cell);
    });

    std::sort(cells.begin(), cells.end(),
              [](auto a, auto b) { return a->r < b->r; });

    for (auto& cell : cells) {
        RGBColor color;

        if (cell->type == t0)
            color = RGBColor(255, 0, 0);
        else if (cell->type == t1)
            color = RGBColor(0, 0, 255);
        else
            color = COLORS[cell->type % (sizeof(COLORS) / sizeof(RGBColor))];

        cairo_set_source_color(ctx, color);
        cairo_circle(ctx, cell->x, cell->y, cell->r);
        cairo_fill(ctx);
    }

    cairo_destroy(ctx);

    unsigned char* data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);

    if (stride != DIM * 4) {
        logger::error("Unexpected stride: %i\n", stride);
        return string_view(nullptr, 0);
    }

    // for visualizing
    // cairo_surface_write_to_png(surface, (__gid + ".png").c_str());

    return string_view((char*)data, stride * height);
}