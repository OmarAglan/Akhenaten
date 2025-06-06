#include "building/building_cattle_ranch.h"

#include "building/building_workshop.h"
#include "graphics/animation.h"
#include "widget/city/ornaments.h"

buildings::model_t<building_cattle_ranch> cattle_ranch_m;

bool building_cattle_ranch::can_play_animation() const {
    if (worker_percentage() < 50) {
        return false;
    }

    return building_industry::can_play_animation();
}

bool building_cattle_ranch::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    draw_normal_anim(ctx, point, tile, color_mask);

    return true;
}
