#include "building/building_pavilion.h"

#include "graphics/image.h"
#include "widget/city/ornaments.h"
#include "widget/city/building_ghost.h"
#include "window/building/common.h"
#include "window/building/figures.h"
#include "sound/sound_building.h"
#include "graphics/elements/ui.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "city/city.h"
#include "city/city_warnings.h"
#include "construction/build_planner.h"
#include "grid/image.h"
#include "grid/building.h"
#include "grid/orientation.h"
#include "grid/building_tiles.h"
#include "figure/figure.h"

building_pavilion::static_params pavilion_m;

void building_pavilion::static_params::load(archive arch) {
    for (auto &pdir : preview_dir) {
        pdir.load(arch, bstring32().printf("preview_dir_%d", std::distance(preview_dir, &pdir)).c_str());
    }

    for (auto &pdir : place_dir) {
        pdir.load(arch, bstring32().printf("place_dir_%d", std::distance(place_dir, &pdir)).c_str());
    }

    dancer_tile = anim[animkeys().base].first_img();
    booth_tile = anim[animkeys().booth].first_img();
    musician_tile_s = anim[animkeys().stand_sn_s].first_img();
    musician_tile_e = anim[animkeys().stand_sn_e].first_img();
}

void building_pavilion::static_params::preview_offset::load(archive arch, pcstr section) {
    arch.r_section(section, [this] (archive d_arch) {
        stand = d_arch.r_vec2i("stand");
        stand_b = d_arch.r_vec2i("stand_b");
        stand_e = d_arch.r_vec2i("stand_e");
        booth = d_arch.r_vec2i("booth");
        stand_b_img = d_arch.r_int("stand_b_img");
        stand_e_img = d_arch.r_int("stand_e_img");
    });
}

void building_pavilion::static_params::place_offset::load(archive arch, pcstr section) {
    arch.r_array(section, items, [this] (archive d_arch, auto &item) {
        item.load(d_arch);
    });
}

void building_pavilion::static_params::place_offset::item::load(archive arch) {
    main = arch.r_bool("main");
    offset = arch.r_vec2i("offset");
    type = arch.r_type<e_building_type>("type");
}


void building_pavilion::static_params::planer_setup_preview_graphics(build_planner &planer) const {
    planer.init_tiles(4, 4);
}

void building_pavilion::static_params::planer_ghost_preview(build_planner &planer, painter &ctx, tile2i start, tile2i end, vec2i pixel) const {
    int can_build = 0;

    int orientation = 0;

    can_build = map_orientation_for_venue_with_map_orientation(end, e_venue_mode_pavilion, &orientation);
    // TODO: proper correct for map orientation (for now, just use a different orientation)
    orientation = abs(orientation + (8 - city_view_orientation())) % 8;

    if (can_build != 1) { // no can place
        for (int i = 0; i < building_size * building_size; i++) {
            planer.draw_flat_tile(ctx, pixel + VIEW_OFFSETS[i], COLOR_MASK_RED);
        }
    } else { // can place (theoretically)
        const auto &params = building_impl::params(BUILDING_PAVILLION);
        int square_id = params.anim[animkeys().square].first_img();
        for (int i = 0; i < building_size * building_size; i++) {
            ImageDraw::isometric(ctx, square_id + i, pixel + vec2i{ ((i % building_size) - (i / building_size)) * 30, ((i % building_size) + (i / building_size)) * 15 }, COLOR_MASK_GREEN);
        }
        int stand_sn_n = params.anim[animkeys().stand_sn_n].first_img();
        int stand_sn_s = params.anim[animkeys().stand_sn_s].first_img();
        int booth = params.anim[animkeys().booth].first_img();
        int stand = params.anim[animkeys().base].first_img();
        const auto &preview_conf = pavilion_m.preview_dir[orientation];

        build_planner::draw_building_ghost(ctx, stand, pixel + preview_conf.stand, COLOR_MASK_GREEN);
        build_planner::draw_building_ghost(ctx, stand_sn_n + preview_conf.stand_b_img, pixel + preview_conf.stand_b, COLOR_MASK_GREEN);
        build_planner::draw_building_ghost(ctx, stand_sn_s + preview_conf.stand_e_img, pixel + preview_conf.stand_e, COLOR_MASK_GREEN);
        build_planner::draw_building_ghost(ctx, booth, pixel + preview_conf.booth, COLOR_MASK_GREEN);
    }
}

void building_pavilion::on_create(int orientation) {

}

void building_pavilion::update_month() {
    runtime_data().play_index = std::rand() % 10;
}

void building_pavilion::update_day() {
    building_impl::update_day();

    auto &d = runtime_data();

    int shows = 0;
    auto update_shows = [&] (auto &days) { if (days > 0) { --days; ++shows; } };
    update_shows(d.juggler_visited);
    update_shows(d.musician_visited);
    update_shows(d.dancer_visited);

    d.num_shows = shows;
}

void building_pavilion::on_place_update_tiles(int orientation, int variant) {
    runtime_data().booth_corner_grid_offset = tile().grid_offset();
    base.orientation = orientation;

    int size = pavilion_m.building_size;
    int image_id = anim(animkeys().square).first_img();

    int basic_orientation;
    map_orientation_for_venue_with_map_orientation(tile(), e_venue_mode_pavilion, &basic_orientation);

    // add underlying plaza first
    map_add_venue_plaza_tiles(id(), size, tile(), image_id, false);
    int absolute_orientation = abs(basic_orientation + (8 - city_view_orientation())) % 8;
    for (const auto &item: pavilion_m.place_dir[absolute_orientation].items) {
        place_latch_on_venue(item.type, item.offset.x, item.offset.y, orientation, item.main);
    }
}

void building_pavilion::on_place_checks() {
    construction_warnings warnings;
    const bool has_dance_school = g_city.buildings.count_active(BUILDING_DANCE_SCHOOL) > 0;
    const bool has_jugglers = g_city.buildings.count_active(BUILDING_JUGGLER_SCHOOL) > 0;
    const bool has_musician = g_city.buildings.count_active(BUILDING_CONSERVATORY) > 0;

    warnings.add_if(!has_dance_school, "#build_dance_school");
    warnings.add_if(!has_jugglers, "#build_juggling_school");
    warnings.add_if(!has_musician, "#build_conservatory");
}

bool building_pavilion::draw_ornaments_and_animations_height(painter &ctx, vec2i point, tile2i tile, color color_mask) {
    auto &d = runtime_data();
    if (d.dancer_visited && map_image_at(tile) == pavilion_m.dancer_tile) {
        const animation_t &ranim = anim(animkeys().dancer);
        building_draw_normal_anim(ctx, point, &base, tile, ranim, color_mask);
    }

    if (d.musician_visited) {
        if (map_image_at(tile) == pavilion_m.musician_tile_s) {
            const animation_t &ranim = anim(animkeys().musician_sn);
            building_draw_normal_anim(ctx, point, &base, tile, ranim, color_mask);
        } else if (map_image_at(tile) == pavilion_m.musician_tile_e) {
            const animation_t &ranim = anim(animkeys().musician_we);
            building_draw_normal_anim(ctx, point, &base, tile, ranim, color_mask);
        }
    }

    if (d.juggler_visited && map_image_at(tile) == pavilion_m.booth_tile) {
        const animation_t &ranim = anim(animkeys().juggler);
        building_draw_normal_anim(ctx, point, &base, tile, ranim, color_mask);
    }

    return true;
}

void building_pavilion::spawn_figure() {
    if (!is_main()) {
        return;
    }

    auto &d = runtime_data();
    if (common_spawn_figure_trigger(100, BUILDING_SLOT_JUGGLER)) {
        if (d.juggler_visited > 0) {
            base.create_roaming_figure(FIGURE_JUGGLER, FIGURE_ACTION_94_ENTERTAINER_ROAMING, BUILDING_SLOT_JUGGLER);
            return;
        }
    }

    if (common_spawn_figure_trigger(100, BUILDING_SLOT_MUSICIAN)) {
        if (d.musician_visited > 0) {
            base.create_roaming_figure(FIGURE_MUSICIAN, FIGURE_ACTION_94_ENTERTAINER_ROAMING, BUILDING_SLOT_MUSICIAN);
            return;
        }
    }

    if (common_spawn_figure_trigger(100, BUILDING_SLOT_DANCER)) {
        if (d.dancer_visited > 0) {
            base.create_roaming_figure(FIGURE_DANCER, FIGURE_ACTION_94_ENTERTAINER_ROAMING, BUILDING_SLOT_DANCER);
            return;
        }
    }
}

void building_pavilion::on_undo() {
    auto &d = runtime_data();
    for (int dy = 0; dy < 4; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            const uint32_t offset = d.booth_corner_grid_offset + GRID_OFFSET(dx, dy);
            if (map_building_at(offset) == 0)
                map_building_set(offset, id());
        }
    }
}

void building_pavilion::update_map_orientation(int map_orientation) {
    if (!is_main()) {
        return;
    }

    int plaza_image_id = anim("square").first_img();
    tile2i btile(runtime_data().booth_corner_grid_offset);
    map_add_venue_plaza_tiles(id(), size(), btile, plaza_image_id, true);
}
