#include "game/gui/filler.h"
#include "game/gui/widget.h"

component *filler_create(void) {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_disable = 1;
    c->supports_select = 0;
    c->supports_focus = 0;
    c->filler = true;
    return c;
}
