#include "formats/sprite.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include <stdlib.h>

void sprite_create(sprite *sp, void *src, int id) {
    memset(sp, 0, sizeof *sp);

    sd_sprite *sdsprite = (sd_sprite *)src;
    sp->id = id;
    sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);

    if(sdsprite->data) {
        // Load data
        sp->ownership = SpriteOwnership_Owned;
        sp->data_private = omf_calloc(1, sizeof(surface));
        sd_vga_image raw;
        sd_sprite_vga_decode(&raw, sdsprite);
        surface_create_from_data(sp->data_private, raw.w, raw.h, (unsigned char *)raw.data);
        sd_vga_image_free(&raw);
    } else {
        // defer load
        sp->ownership = SpriteOwnership_SdSprite;
        sp->data_private = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_copy(sp->data_private, sdsprite);
    }
}

void sprite_create_reference(sprite *sp, void *src, int id, void *data) {
    memset(sp, 0, sizeof *sp);
    sd_sprite *sdsprite = (sd_sprite *)src;
    sp->id = id;
    sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
    sp->data_private = data;
    sp->ownership = SpriteOwnership_Borrowed;
}

void sprite_free(sprite *sp) {
    if(sp->ownership = SpriteOwnership_Owned) {
        surface_free(sp->data_private);
        omf_free(sp->data_private);
    } else if(sp->ownership = SpriteOwnership_SdSprite) {
        sd_sprite_free(sp->data_private);
    }
}

vec2i sprite_get_size(sprite *sp) {
    surface *surf = sprite_get_surface(sp);
    if(surf != NULL) {
        return vec2i_create(surf->w, surf->h);
    }
    return vec2i_create(0, 0);
}

surface *sprite_get_surface(sprite *s) {
    switch(s->ownership) {
        case SpriteOwnership_Owned:
        case SpriteOwnership_Borrowed:
            return s->data_private;
        case SpriteOwnership_SdSprite: {
            sd_sprite *sdspr = s->data_private;

            s->ownership = SpriteOwnership_Owned;
            s->data_private = omf_calloc(1, sizeof(surface));

            // Load data
            sd_vga_image raw;
            sd_sprite_vga_decode(&raw, sdspr);
            surface_create_from_data(s->data_private, raw.w, raw.h, (unsigned char *)raw.data);
            sd_vga_image_free(&raw);

            sd_sprite_free(sdspr);
            return s->data_private;
        }
    }
    return NULL;
}

sprite *sprite_copy(sprite *src) {
    if(src == NULL)
        return NULL;

    sprite *new = omf_calloc(1, sizeof(sprite));
    new->pos = src->pos;
    new->id = src->id;

    // Copy surface
    new->data_private = omf_calloc(1, sizeof(surface));
    surface_create_from(new->data_private, sprite_get_surface(src));
    new->ownership = SpriteOwnership_Owned;
    return new;
}
