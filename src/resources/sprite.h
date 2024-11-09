#ifndef SPRITE_H
#define SPRITE_H

#include "utils/vec.h"
#include "video/surface.h"

typedef enum sprite_ownership_e
{
    SpriteOwnership_Owned = 1,
    SpriteOwnership_Borrowed,
    SpriteOwnership_SdSprite,
} sprite_ownership;

typedef struct sprite_t {
    int id;
    vec2i pos;
    void *data_private;
    sprite_ownership ownership;
} sprite;

void sprite_create(sprite *sp, void *src, int id);
void sprite_create_reference(sprite *sp, void *src, int id, void *data);
void sprite_free(sprite *sp);

vec2i sprite_get_size(sprite *s);
surface *sprite_get_surface(sprite *s);
sprite *sprite_copy(sprite *src);

#endif // SPRITE_H
