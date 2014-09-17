#include "game/menu/label.h"
#include "game/menu/widget.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    font *font;
} label;

void label_render(component *c) {
    label *local = widget_get_obj(c);
    int chars = strlen(local->text);
    int width = chars * local->font->w;
    int xoff = (c->w - width) / 2;
    font_render(local->font, local->text, c->x + xoff, c->y, color_create(121, 121, 121, 255));
}

void label_free(component *c) {
    label *local = widget_get_obj(c);
    free(local->text);
    free(local);
}

component* label_create(font *font, const char *text) {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    label *local = malloc(sizeof(label));
    memset(local, 0, sizeof(label));
    local->font = font;
    local->text = strdup(text);

    widget_set_obj(c, local);
    widget_set_render_cb(c, label_render);
    widget_set_free_cb(c, label_free);

    return c;
}
