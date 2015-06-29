#include "sdl2_surface.h"
#include "sdl2_rect.h"
#include "sdl2_pixels.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/variable.h"

static struct RClass *class_Surface;

typedef struct mrb_sdl2_video_surface_data_t {
  bool         is_associated;
  SDL_Surface *surface;
} mrb_sdl2_video_surface_data_t;

static void
mrb_sdl2_video_surface_data_free(mrb_state *mrb, void *p)
{
  mrb_sdl2_video_surface_data_t* data =
    (mrb_sdl2_video_surface_data_t*)p;
  if (NULL != data) {
    if ((NULL != data->surface) && (false == data->is_associated)) {
      SDL_FreeSurface(data->surface);
    }
    mrb_free(mrb, data);
  }
}

static struct mrb_data_type const mrb_sdl2_video_surface_data_type = {
  "Surface", mrb_sdl2_video_surface_data_free
};

mrb_value
mrb_sdl2_video_surface(mrb_state *mrb, SDL_Surface *surface, bool is_associated)
{
  mrb_sdl2_video_surface_data_t* data =
    (mrb_sdl2_video_surface_data_t*)mrb_malloc(mrb, sizeof(mrb_sdl2_video_surface_data_t));
  if (NULL == data) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "insufficient memory.");
  }
  data->is_associated = is_associated;
  data->surface = surface;
  return mrb_obj_value(Data_Wrap_Struct(mrb, class_Surface, &mrb_sdl2_video_surface_data_type, data));
}

SDL_Surface *
mrb_sdl2_video_surface_get_ptr(mrb_state *mrb, mrb_value surface)
{
  mrb_sdl2_video_surface_data_t *data;
  if (mrb_nil_p(surface)) {
    return NULL;
  }
  data =
    (mrb_sdl2_video_surface_data_t*)mrb_data_get_ptr(mrb, surface, &mrb_sdl2_video_surface_data_type);
  return data->surface;
}


static mrb_value
mrb_sdl2_video_surface_initialize(mrb_state *mrb, mrb_value self)
{
  uint32_t flags, rmask, gmask, bmask, amask;
  mrb_int width, height, depth;
  mrb_sdl2_video_surface_data_t *data =
    (mrb_sdl2_video_surface_data_t*)DATA_PTR(self);
  mrb_get_args(mrb, "iiiiiiii", &flags, &width, &height, &depth, &rmask, &gmask, &bmask, &amask);
  if (NULL == data) {
    data = (mrb_sdl2_video_surface_data_t*)mrb_malloc(mrb, sizeof(mrb_sdl2_video_surface_data_t));
    if (NULL == data) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "insufficient memory.");
    }
    data->surface = NULL;
  } else {
    if (NULL != data->surface) {
      SDL_FreeSurface(data->surface);
    }
  }
  data->surface = SDL_CreateRGBSurface(flags, width, height, depth, rmask, gmask, bmask, amask);
  if (NULL == data->surface) {
    mrb_free(mrb, data);
    mruby_sdl2_raise_error(mrb);
  }
  DATA_PTR(self) = data;
  DATA_TYPE(self) = &mrb_sdl2_video_surface_data_type;
  return self;
}

static mrb_value
mrb_sdl2_video_surface_free(mrb_state *mrb, mrb_value self)
{
  mrb_sdl2_video_surface_data_t *data =
    (mrb_sdl2_video_surface_data_t*)mrb_data_get_ptr(mrb, self, &mrb_sdl2_video_surface_data_type);
  if ((NULL != data->surface) && (false == data->is_associated)) {
    SDL_FreeSurface(data->surface);
    data->surface = NULL;
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_blit_scaled(mrb_state *mrb, mrb_value self)
{
  int ret;
  SDL_Surface * ds;
  SDL_Rect * sr;
  SDL_Surface * ss;
  SDL_Rect * dr;
  mrb_value src_rect, dst, dst_rect;
  mrb_get_args(mrb, "ooo", &src_rect, &dst, &dst_rect);
  ss = mrb_sdl2_video_surface_get_ptr(mrb, self);
  sr = mrb_sdl2_rect_get_ptr(mrb, src_rect);
  ds = mrb_sdl2_video_surface_get_ptr(mrb, dst);
  dr = mrb_sdl2_rect_get_ptr(mrb, dst_rect);
  if (NULL != dr) {
    SDL_Rect tmp = *dr;
    ret = SDL_BlitScaled(ss, sr, ds, &tmp);
  } else {
    ret = SDL_BlitScaled(ss, sr, ds, dr);
  }
  if (0 != ret) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_blit_surface(mrb_state *mrb, mrb_value self)
{
  SDL_Surface * ds;
  SDL_Rect * sr;
  SDL_Surface * ss;
  SDL_Rect * dr;
  SDL_Rect tmp;
  mrb_value src_rect, dst, dst_rect;
  mrb_get_args(mrb, "ooo", &src_rect, &dst, &dst_rect);
  ss = mrb_sdl2_video_surface_get_ptr(mrb, self);
  sr = mrb_sdl2_rect_get_ptr(mrb, src_rect);
  ds = mrb_sdl2_video_surface_get_ptr(mrb, dst);
  dr = mrb_sdl2_rect_get_ptr(mrb, dst_rect);
  if (NULL == dr) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot set 3rd argument nil.");
  }
  tmp = *dr;
  if (0 != SDL_BlitSurface(ss, sr, ds, &tmp)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_convert_format(mrb_state *mrb, mrb_value self)
{
  mrb_raise(mrb, E_NOTIMP_ERROR, "not implemented.");
  return self;
}

static mrb_value
mrb_sdl2_video_surface_format(mrb_state *mrb, mrb_value self)
{
  mrb_value format = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "__pixel_format__"));

  if (mrb_nil_p(format)){
    format = mrb_sdl2_pixels_pixelformat_new(mrb, mrb_sdl2_video_surface_get_ptr(mrb, self)->format);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "__pixel_format__"), format);
  }

  return format;
}

static mrb_value
mrb_sdl2_video_surface_fill_rect(mrb_state *mrb, mrb_value self)
{
  uint32_t color;
  mrb_value rect;
  SDL_Surface *s;
  SDL_Rect * r;
  mrb_get_args(mrb, "i|o", &color, &rect);
  s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  r = mrb_sdl2_rect_get_ptr(mrb, rect);
  if (0 != SDL_FillRect(s, r, color)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_fill_rects(mrb_state *mrb, mrb_value self)
{
  uint32_t color;
  mrb_value rects;
  mrb_int n;
  SDL_Surface *s;
  SDL_Rect * r;
  mrb_int i;
  mrb_get_args(mrb, "io", &color, &rects);
  if (!mrb_array_p(rects)) {
    mrb_raise(mrb, E_TYPE_ERROR, "given 2nd argument is unexpected type (expected Array).");
  }
  n = mrb_ary_len(mrb, rects);
  s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  r = (SDL_Rect *) SDL_malloc(sizeof(SDL_Rect) * n);
  for (i = 0; i < n; ++i) {
    SDL_Rect const * const ptr = mrb_sdl2_rect_get_ptr(mrb, mrb_ary_ref(mrb, rects, i));
    if (NULL != ptr) {
      r[i] = *ptr;
    } else {
      r[i] = (SDL_Rect){ 0, 0, 0, 0 };
    }
  }
  if (0 != SDL_FillRects(s, r, n, color)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_get_clip_rect(mrb_state *mrb, mrb_value self)
{
  SDL_Rect rect;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  SDL_GetClipRect(s, &rect);
  return mrb_sdl2_rect_direct(mrb, &rect);
}

static mrb_value
mrb_sdl2_video_surface_set_clip_rect(mrb_state *mrb, mrb_value self)
{
  SDL_Rect * rect;
  mrb_value arg;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  mrb_get_args(mrb, "o", &arg);
  rect = mrb_sdl2_rect_get_ptr(mrb, arg);
  if (SDL_FALSE == SDL_SetClipRect(s, rect)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_get_color_key(mrb_state *mrb, mrb_value self)
{
  uint32_t key;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_GetColorKey(s, &key)) {
    mruby_sdl2_raise_error(mrb);
  }
  return mrb_fixnum_value(key);
}

static mrb_value
mrb_sdl2_video_surface_get_solor_key(mrb_state *mrb, mrb_value self)
{
  uint32_t key;
  int flag;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  mrb_get_args(mrb, "ii", &flag, &key);
  if (0 != SDL_SetColorKey(s, flag, key)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_get_alpha_mod(mrb_state *mrb, mrb_value self)
{
  uint8_t alpha;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_GetSurfaceAlphaMod(s, &alpha)) {
    mruby_sdl2_raise_error(mrb);
  }
  return mrb_fixnum_value(alpha);
}

static mrb_value
mrb_sdl2_video_surface_set_alpha_mod(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *s;
  mrb_int alpha;
  mrb_get_args(mrb, "i", &alpha);
  s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_SetSurfaceAlphaMod(s, (uint8_t)(alpha & 0xff))) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_get_blend_mode(mrb_state *mrb, mrb_value self)
{
  SDL_BlendMode mode;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_GetSurfaceBlendMode(s, &mode)) {
    mruby_sdl2_raise_error(mrb);
  }
  return mrb_fixnum_value(mode);
}

static mrb_value
mrb_sdl2_video_surface_set_blend_mode(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *s;
  mrb_int mode;
  mrb_get_args(mrb, "i", &mode);
  s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_SetSurfaceBlendMode(s, (SDL_BlendMode)mode)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_get_color_mod(mrb_state *mrb, mrb_value self)
{
  mrb_value rgb[3];
  uint8_t r, g, b;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_GetSurfaceColorMod(s, &r, &g, &b)) {
    mruby_sdl2_raise_error(mrb);
  }
  rgb[0] = mrb_fixnum_value(r);
  rgb[1] = mrb_fixnum_value(g);
  rgb[2] = mrb_fixnum_value(b);

  return mrb_obj_new(mrb, mrb_class_get_under(mrb, mod_SDL2, "RGB"), 3, rgb);
}

static mrb_value
mrb_sdl2_video_surface_set_color_mod(mrb_state *mrb, mrb_value self)
{
  uint8_t r, g, b;
  mrb_value color;
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  mrb_get_args(mrb, "o", &color);
  if (!mrb_obj_is_kind_of(mrb, color, mrb_class_get_under(mrb, mod_SDL2, "RGB"))) {
    mrb_raise(mrb, E_TYPE_ERROR, "given 1st argument is unexpected type (expected RGB).");
  }
  r = mrb_fixnum(mrb_iv_get(mrb, color, mrb_intern(mrb, "@r", 2)));
  g = mrb_fixnum(mrb_iv_get(mrb, color, mrb_intern(mrb, "@g", 2)));
  b = mrb_fixnum(mrb_iv_get(mrb, color, mrb_intern(mrb, "@b", 2)));
  if (0 != SDL_SetSurfaceColorMod(s, r, g, b)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_set_palette(mrb_state *mrb, mrb_value self)
{
  mrb_raise(mrb, E_NOTIMP_ERROR, "not implemented.");
  return self;
}

static mrb_value
mrb_sdl2_video_surface_set_rle(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  mrb_bool is_rle_enabled;
  mrb_get_args(mrb, "b", &is_rle_enabled);
  if (0 != SDL_SetSurfaceRLE(s, is_rle_enabled ? 1 : 0)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_lock(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  if (0 != SDL_LockSurface(s)) {
    mruby_sdl2_raise_error(mrb);
  }
  return self;
}

static mrb_value
mrb_sdl2_video_surface_unlock(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *s = mrb_sdl2_video_surface_get_ptr(mrb, self);
  SDL_UnlockSurface(s);
  return self;
}


/*
 * SDL2::Video::Surface::load_bmp
 */
static mrb_value
mrb_sdl2_video_surface_load_bmp(mrb_state *mrb, mrb_value self)
{
  SDL_Surface *surface;
  mrb_value file;
  mrb_get_args(mrb, "S", &file);
  surface = SDL_LoadBMP(RSTRING_PTR(file));
  if (NULL == surface) {
    mruby_sdl2_raise_error(mrb);
  }
  return mrb_sdl2_video_surface(mrb, surface, false);
}

/*
 * SDL2::Video::Surface::save_bmp
 */
static mrb_value
mrb_sdl2_video_surface_save_bmp(mrb_state *mrb, mrb_value self)
{
  SDL_Surface * s;
  mrb_value surface, file;
  mrb_get_args(mrb, "oo", &surface, &file);
  s = mrb_sdl2_video_surface_get_ptr(mrb, surface);
  if (0 != SDL_SaveBMP(s, RSTRING_PTR(file))) {
    mruby_sdl2_raise_error(mrb);
  }
  return mrb_nil_value();
}

static mrb_value
mrb_sdl2_video_surface_map_rgba(mrb_state *mrb, mrb_value self)
{
  SDL_Surface * s;
  Uint32 result;
  mrb_value surface;
  mrb_int r,g,b,a;
  mrb_get_args(mrb, "oiiii", &surface, &r, &g, &b, &a);
  s = mrb_sdl2_video_surface_get_ptr(mrb, surface);
  result = SDL_MapRGBA(s->format, r, g, b, a);
  return mrb_fixnum_value(result);
}

static mrb_value
mrb_sdl2_video_surface_map_rgb(mrb_state *mrb, mrb_value self)
{
  SDL_Surface * s;
  Uint32 result;
  mrb_value surface;
  mrb_int r,g,b;
  mrb_get_args(mrb, "oiii", &surface, &r, &g, &b);
  s = mrb_sdl2_video_surface_get_ptr(mrb, surface);
  result = SDL_MapRGB(s->format, r, g, b);
  return mrb_fixnum_value(result);
}

void
mruby_sdl2_video_surface_init(mrb_state *mrb, struct RClass *mod_Video)
{
  class_Surface = mrb_class_get_under(mrb, mod_Video, "Surface");

  MRB_SET_INSTANCE_TT(class_Surface, MRB_TT_DATA);

  mrb_define_method(mrb, class_Surface, "initialize",     mrb_sdl2_video_surface_initialize,     ARGS_REQ(8));
  mrb_define_method(mrb, class_Surface, "free",           mrb_sdl2_video_surface_free,           ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "destroy",        mrb_sdl2_video_surface_free,           ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "blit_scaled",    mrb_sdl2_video_surface_blit_scaled,    ARGS_REQ(2) | ARGS_OPT(1));
  mrb_define_method(mrb, class_Surface, "blit_surface",   mrb_sdl2_video_surface_blit_surface,   ARGS_REQ(3));
  mrb_define_method(mrb, class_Surface, "convert_format", mrb_sdl2_video_surface_convert_format, ARGS_REQ(2));

  mrb_define_method(mrb, class_Surface, "format",         mrb_sdl2_video_surface_format,         ARGS_NONE());

  mrb_define_method(mrb, class_Surface, "fill_rect",      mrb_sdl2_video_surface_fill_rect,      ARGS_REQ(1) | ARGS_OPT(1));
  mrb_define_method(mrb, class_Surface, "fill_rects",     mrb_sdl2_video_surface_fill_rects,     ARGS_REQ(2));
  mrb_define_method(mrb, class_Surface, "clip_rect",      mrb_sdl2_video_surface_get_clip_rect,  ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "clip_rect=",     mrb_sdl2_video_surface_set_clip_rect,  ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "color_key_get",  mrb_sdl2_video_surface_get_color_key,  ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "color_key_set",  mrb_sdl2_video_surface_get_solor_key,  ARGS_REQ(2));
  mrb_define_method(mrb, class_Surface, "alpha_mod",      mrb_sdl2_video_surface_get_alpha_mod,  ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "alpha_mod=",     mrb_sdl2_video_surface_set_alpha_mod,  ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "blend_mode",     mrb_sdl2_video_surface_get_blend_mode, ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "blend_mode=",    mrb_sdl2_video_surface_set_blend_mode, ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "color_mod",      mrb_sdl2_video_surface_get_color_mod,  ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "color_mod=",     mrb_sdl2_video_surface_set_color_mod,  ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "palette",        mrb_sdl2_video_surface_set_palette,    ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "rle",            mrb_sdl2_video_surface_set_rle,        ARGS_REQ(1));
  mrb_define_method(mrb, class_Surface, "lock",           mrb_sdl2_video_surface_lock,           ARGS_NONE());
  mrb_define_method(mrb, class_Surface, "unlock",         mrb_sdl2_video_surface_unlock,         ARGS_NONE());

  mrb_define_class_method(mrb, class_Surface, "load_bmp", mrb_sdl2_video_surface_load_bmp, ARGS_REQ(1));
  mrb_define_class_method(mrb, class_Surface, "save_bmp", mrb_sdl2_video_surface_save_bmp, ARGS_REQ(2));
  mrb_define_class_method(mrb, class_Surface, "map_rgba", mrb_sdl2_video_surface_map_rgba, ARGS_REQ(4));
  mrb_define_class_method(mrb, class_Surface, "map_rgb",  mrb_sdl2_video_surface_map_rgb,  ARGS_REQ(4));
}

void
mruby_sdl2_video_surface_final(mrb_state *mrb, struct RClass *mod_Video)
{
}
