#include "city_festival.h"

#include "building/building_type.h"
#include "building/building_storage_yard.h"
#include "buildings.h"
#include "city/constants.h"
#include "city/city.h"
#include "city/finance.h"
#include "city/city_events.h"
#include "city/city_message.h"
#include "core/random.h"
#include "figure/figure.h"
#include "game/game_config.h"
#include "graphics/image_groups.h"

bool city_festival_t::is_planned() {
    return planned.size != FESTIVAL_NONE;
}

int city_festival_t::months_till_next() {
    return planned.months_to_go;
}

int city_festival_t::selected_god() {
    return selected.god;
}
void city_festival_t::select_god(e_god god_id) {
    selected.god = god_id;
}

int city_festival_t::selected_size() {
    return selected.size;
}

bool city_festival_t::select_size(e_festival_type size) {
    if (size == FESTIVAL_GRAND && not_enough_alcohol) {
        return false;
    }

    selected.size = size;
    return true;
}

void city_festival_t::schedule() {
    planned.god = selected.god;
    planned.size = selected.size;

    int cost;
    int population = g_city.population.current;
    switch (selected.size) {
    case FESTIVAL_SMALL:
        planned.months_to_go = small_min_months + population / 1000 + 1;
        cost = small_cost;
        break;

    case FESTIVAL_LARGE:
        planned.months_to_go = middle_min_months + population / 1500 + 1;
        cost = large_cost;
        break;

    default:
        planned.months_to_go = grand_min_minths + population / 2000 + 1;
        cost = grand_cost;
    }

    city_finance_process_requests_and_festivals(cost);

    if (selected.size == FESTIVAL_GRAND) {
        events::emit(event_storageyards_remove_resource{ RESOURCE_BEER, grand_alcohol });
    }
}

void city_festival_t::execute_festival() {
    tile2i square_pos = city_building_get_festival_square_position();
    building *square = building_at(square_pos);
    if (!square->is_valid()) {
        return;
    }

    for (int i = 1; i < MAX_BUILDINGS; i++) {
        building* b = building_get(i);
        if (b->state != BUILDING_STATE_VALID)
            continue;

        switch (b->type) {
        case BUILDING_TEMPLE_OSIRIS:
        case BUILDING_TEMPLE_COMPLEX_OSIRIS:
        case BUILDING_TEMPLE_RA:
        case BUILDING_TEMPLE_COMPLEX_RA:
        case BUILDING_TEMPLE_PTAH:
        case BUILDING_TEMPLE_COMPLEX_PTAH:
        case BUILDING_TEMPLE_SETH:
        case BUILDING_TEMPLE_COMPLEX_SETH:
        case BUILDING_TEMPLE_BAST:
        case BUILDING_TEMPLE_COMPLEX_BAST:
        case BUILDING_JUGGLER_SCHOOL:
        case BUILDING_CONSERVATORY:
        case BUILDING_DANCE_SCHOOL: { // this actually doesn't happen in Pharaoh?
                                      //                    if (b->has_figure(0))
                                      //                        b->remove_figure(0);
                figure* f = b->create_figure_generic(FIGURE_FESTIVAL_PRIEST, FIGURE_ACTION_10_FESTIVAL_PRIEST_CREATED, BUILDING_SLOT_PRIEST, DIR_4_BOTTOM_LEFT);
                
                tile2i tile_on_square = square_pos.shifted(rand() % square->size, rand() % square->size);
                f->tile = b->road_access;
                f->set_destination(square);
                f->destination_tile = tile_on_square;
                f->festival_remaining_dances = rand() % 10;
                f->wait_ticks = rand() % 10;
            }
            break;
        }
    }

    if (first_festival_effect_months <= 0) {
        first_festival_effect_months = 12;
        switch (planned.size) {
        case FESTIVAL_SMALL:
            g_city.change_happiness(7);
            break;
        case FESTIVAL_LARGE:
            g_city.change_happiness(9);
            break;
        case FESTIVAL_GRAND:
            g_city.change_happiness(12);
            break;
        }
    } else if (second_festival_effect_months <= 0) {
        second_festival_effect_months = 12;
        switch (planned.size) {
        case FESTIVAL_SMALL:
            g_city.change_happiness(2);
            break;
        case FESTIVAL_LARGE:
            g_city.change_happiness(3);
            break;
        case FESTIVAL_GRAND:
            g_city.change_happiness(5);
            break;
        }
    } else {
        switch (planned.size) {
        case FESTIVAL_SMALL:
            g_city.change_happiness(1);
            break;
        
        case FESTIVAL_LARGE:
            g_city.change_happiness(2);
            break;
        
        case FESTIVAL_GRAND:
            g_city.change_happiness(3);
            break;
        }
    }

    months_since_festival = 1;

    g_city.religion.gods[planned.god].months_since_festival = 0;
    switch (planned.size) {
    case FESTIVAL_SMALL:
        messages::popup(MESSAGE_SMALL_FESTIVAL, 0, 0);
        break;

    case FESTIVAL_LARGE:
        messages::popup(MESSAGE_LARGE_FESTIVAL, 0, 0);
        break;

    case FESTIVAL_GRAND:
        messages::popup(MESSAGE_GRAND_FESTIVAL, 0, 0);

        if (!!game_features::gameplay_change_grandfestival) {
            g_city.religion.gods[planned.god].blessing_done = 0;
        }

        break;
    }

    planned.size = FESTIVAL_NONE;
    planned.months_to_go = 0;
}

void city_festival_t::update() {
    months_since_festival = std::min<uint8_t>(60, months_since_festival++);

    if (first_festival_effect_months > 0) {
        --first_festival_effect_months;
    }

    if (second_festival_effect_months > 0) {
        --second_festival_effect_months;
    }

    if (!is_planned()) {
        return;
    }
    
    planned.months_to_go--;
    if (planned.months_to_go <= 0) {
        planned.months_to_go = 0;
        execute_festival();
    }
}

void city_festival_t::calculate_costs() {
    int population = g_city.population.current;
    small_cost = population / 20 + 10;
    large_cost = population / 10 + 20;
    grand_cost = population / 5 + 40;
    grand_alcohol = population / 50 + 1;
    not_enough_alcohol = false;

    if (g_city.resource.yards_stored(RESOURCE_BEER) < grand_alcohol) {
        not_enough_alcohol = true;
        if (selected.size == FESTIVAL_GRAND) {
            selected.size = FESTIVAL_LARGE;
        }
    }
}

int city_festival_t::get_advice() {
    if (months_since_festival <= 1)
        return 0;
    else if (months_since_festival <= 6)
        return 1;
    else if (months_since_festival <= 12)
        return 2;
    else if (months_since_festival <= 18)
        return 3;
    else if (months_since_festival <= 24)
        return 4;
    else if (months_since_festival <= 30)
        return 5;
    else {
        return 6;
    }
}

