
#ifndef includeguard_display_hpp_includeguard
#define includeguard_display_hpp_includeguard

#include <memory>

class pixel_format;

struct window;
struct display;

struct window
{
  virtual ~window (void) { }
  virtual void set_title (const char* val) = 0;
  virtual void close (void) = 0;
  virtual void* handle (void) const = 0;
  virtual void show (void) = 0;
  virtual bool process_events (void) = 0;
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
