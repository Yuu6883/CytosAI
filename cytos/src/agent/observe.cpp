#include "../entity/control.hpp"
#include "../physics/engine.hpp"
#include "agent.hpp"

constexpr int32_t DIM = 256;

// static thread_local sk_sp<SkSurface> surface = nullptr;

// constexpr SkColor COLORS[12] = {
//     SkColorSetRGB(0, 255, 148),   SkColorSetRGB(132, 255, 61),
//     SkColorSetRGB(255, 152, 67),  SkColorSetRGB(0, 240, 234),
//     SkColorSetRGB(255, 0, 98),    SkColorSetRGB(0, 169, 255),
//     SkColorSetRGB(255, 226, 0),   SkColorSetRGB(148, 0, 255),
//     SkColorSetRGB(196, 4, 78),    SkColorSetRGB(192, 255, 58),
//     SkColorSetRGB(255, 106, 228), SkColorSetRGB(255, 142, 0),
// };

string_view Agent::observe() {
    if (!control || !dual || !dual->control) return string_view(nullptr, 0);

    // if (surface == nullptr) {
    //     surface = SkSurface::MakeRasterN32Premul(DIM, DIM, nullptr);
    // }

    uint16_t t0 = control->id;
    uint16_t t1 = dual->control->id;

    // Get a canvas object from the surface
    // SkCanvas* canvas = surface->getCanvas();

    auto vl = std::max(control->viewport.hw, control->viewport.hh);
    auto& v = control->viewport;
    // SkRect view_rect = SkRect::MakeXYWH(v.x - vl, v.y - vl, vl * 2, vl * 2);
    // SkRect screen_rect = SkRect::MakeXYWH(0, 0, DIM, DIM);

    // SkMatrix proj;
    // bool success = proj.setRectToRect(view_rect, screen_rect,
    //                                   SkMatrix::ScaleToFit::kCenter_ScaleToFit);

    // if (!success) {
    //     logger::error("matrix projection failed\n");
    //     return string_view(nullptr, 0);
    // }

    auto map = engine->getMap();

    /*
    SkRect map_rect;
    proj.mapRect(map_rect);

    // Draw background
    canvas->clear(SK_ColorBLACK);
    // Draw map

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SkColorSetRGB(17, 17, 17));
    paint.setStyle(SkPaint::kFill_Style);

    canvas->drawRect(map_rect, paint);
    paint.setColor(SkColorSetRGB(255, 255, 255));

    */
    auto aabb = v.toAABB();

    // Query pellets, no need to sort
    engine->queryGridPL(aabb, [&](Cell* cell) {
        // SkPoint center = proj.mapXY(cell->x, cell->y);
        // SkScalar radius = proj.mapRadius(cell->r);
        // canvas->drawCircle(center.x(), center.y(), radius, paint);
    });

    // Query ejected + virus, no need to sort(?)
    engine->queryGridEV(aabb, [&](Cell* cell) {
        // SkPoint center = proj.mapXY(cell->x, cell->y);
        // SkScalar radius = proj.mapRadius(cell->r);

        // paint.setColor(cell->type & EJECT_BIT ? SkColorSetRGB(200, 200, 200)
        //                                       : SkColorSetRGB(0, 255, 0));
        // canvas->drawCircle(center.x(), center.y(), radius, paint);
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
        // SkPoint center = proj.mapXY(cell->x, cell->y);
        // SkScalar radius = proj.mapRadius(cell->r);

        // SkColor color;
        // if (cell->type == t0)
        //     color = SkColorSetRGB(255, 0, 0);
        // else if (cell->type == t1)
        //     color = SkColorSetRGB(0, 0, 255);
        // else
        //     color = COLORS[cell->type % (sizeof(COLORS) / sizeof(SkColor))];

        // paint.setColor(color);
        // canvas->drawCircle(center.x(), center.y(), radius, paint);
    }

    // canvas->flush();

    // auto img = surface->makeImageSnapshot();

    // SkPixmap pixmap;
    // success = img->peekPixels(&pixmap);

    // if (!success) {
    //     logger::error("peekPixels failed\n");
    //     return string_view(nullptr, 0);
    // }

    // {
    //     auto data = img->encodeToData(SkEncodedImageFormat::kJPEG, 100);
    //     SkFILEWStream file((__gid + ".jpeg").c_str());
    //     file.write(data->data(), data->size());
    // }

    // return string_view(static_cast<char*>(pixmap.writable_addr()),
    //                    pixmap.computeByteSize());

    return string_view(nullptr, 0);
}