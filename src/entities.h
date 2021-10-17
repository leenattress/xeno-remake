typedef struct
{
  int x;
  int y;
  int w;
  int h;
  int sleep;
  int velx;
  int vely;
  int health;
  int active;
  int type;
  int control;
  Sprite *sprite;
  char name[6];
} Entity;

void entity_kill(Entity *e)
{
  e->active = FALSE;
  e->health = 0;
  SPR_setVisibility(e->sprite, HIDDEN);
}

void entity_revive(Entity *e)
{
  e->active = TRUE;
  e->health = 1;
  SPR_setVisibility(e->sprite, VISIBLE);
}

int entities_collide(Entity *a, Entity *b)
{
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h >= b->y);
}