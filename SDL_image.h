#ifndef SDL_image_H
#define SDL_image_H


#define IMG_INIT_PNG 0


#ifdef _MAC
extern SDL_Surface *IMG_Load(const char *n);
extern const char *IMG_GetError(void);
extern int IMG_Init(int val);
#endif


#endif