
#ifndef includeguard_display_hpp_includeguard
#define includeguard_display_hpp_includeguard

#include <memory>
#include <functional>
#include "utils/vec_mat.hpp"

namespace img
{
class pixel_format;
}

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

  enum key_code_t
  {
    no_key,
    key_f1,
    key_f2,
    key_f3,
    key_f4,
    key_f5,
    key_f6,
    key_f7,
    key_f8,
    key_f9,
    key_f10,
    key_f11,
    key_f12,
    key_esc,
    key_space
  };

  enum button_t
  {
    no_button = 0,
    button_left = 1,
    button_right = 3
  };

  type_t type = unknown;
  utils::vec2<int> pos = { 0 };
  utils::vec2<int> drag_start_pos = { 0 };
  utils::vec2<int> drag_delta = { 0 };
  utils::vec2<int> drag_abs = { 0 };
  int button = no_button;
  int keycode = no_key;

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
  virtual bool process_events (void) = 0;

  virtual void set_input_event_clb (const std::function<void (const input_event&)>& clb) = 0;

  virtual utils::vec2<int> size (void) const = 0;
  virtual utils::vec2<int> client_size (void) const = 0;
};

struct display
{
  virtual ~display (void) { }
  virtual void* handle (void) const = 0;
  virtual std::unique_ptr<window> create_window (int visual_id, int width, int height) = 0;

  // implemented by the concrete device.
  static std::unique_ptr<display> make_new (img::pixel_format, int swapinterval, int multisample);
};

#endif // includeguard_display_hpp_includeguard
