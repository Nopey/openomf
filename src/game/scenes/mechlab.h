#ifndef MECHLAB_H
#define MECHLAB_H

#include "formats/chr.h"
#include "game/protos/scene.h"
#include "game/gui/component.h"

typedef enum
{
    DASHBOARD_NONE,
    DASHBOARD_STATS,
    DASHBOARD_NEW,
    DASHBOARD_SELECT_NEW_PIC,
    DASHBOARD_SELECT_DIFFICULTY,
    DASHBOARD_SELECT_TOURNAMENT,
} dashboard_type;

int mechlab_create(scene *scene);
void mechlab_update(scene *scene);

void mechlab_enter_trnselect_menu(scene *s);
component *mechlab_chrload_menu_create(scene *scene);
component *mechlab_chrdelete_menu_create(scene *scene);

void mechlab_select_dashboard(scene *scene, dashboard_type type);

void mechlab_set_selling(scene *scene, bool selling);
bool mechlab_get_selling(scene *scene);

void mechlab_set_hint(scene *scene, const char *hint);

sd_chr_enemy *mechlab_next_opponent(scene *scene);

#endif // MECHLAB_H
