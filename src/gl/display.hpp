
#ifndef includeguard_display_hpp_includeguard
#define includeguard_display_hpp_includeguard

#include <memory>
#include <functional>
#include "utils/vec_mat.hpp"

class pixel_format;

struct window;
struct display;

struct input_event
{
  enum type_t
  {
    unknown = 0,
    mouse_down,
    mouse_up,
    mouse_click,
    mouse_move,
    mouse_drag,
    mouse_wheel,
    key_down,
    key_up,
  };

  type_t type = unknown;
  vec2<int> pos = { 0 };
  vec2<int> drag_start_pos = { 0 };
  vec2<int> drag_delta = { 0 };
  vec2<int> drag_abs = { 0 };
  int button = 0;
  int keycode = 0;

  // -1 = up, 1 = down
  int wheel_delta = 0;
};

struct window
{
  virtual ~window (void) { }
  virtual void set_title (const char* val) = 0;
  virtual void close (void) = 0;
  virtual void* handle (void) const = 0;
  virtual void show (void) = 0;
  virtual bool process_events (const std::function<void (const input_event&)>& clb = [] (auto) { }) = 0;
};

struct display
{
  virtual ~display (void) { }
  virtual void* handle (void) const = 0;
  virtual std::unique_ptr<window> create_window (int visual_id, int width, int height) = 0;

  // implemented by the concrete device.
  static std::unique_ptr<display> make_new (pixel_format, int swapinterval, int multisample);
};

#endif // includeguard_display_hpp_includeguard
