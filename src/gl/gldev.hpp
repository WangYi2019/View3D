
#ifndef includeguard_gldev_hpp_includeguard
#define includeguard_gldev_hpp_includeguard

struct display;
struct window;
struct pixel_format;
struct ds_format;
struct gldev;

struct gldev
{
  virtual ~gldev (void) { }
  virtual bool valid (void) const = 0;
  virtual int native_visual_id (void) const = 0;

  virtual void init_surface (window&) = 0;
  virtual bool surface_valid (void) const = 0;

  virtual void create_context (int swapinterval) = 0;
  virtual bool context_valid (void) const = 0;

  virtual void init_extensions (void) = 0;

  virtual void swap_buffers (void) = 0;

  // implemented by the concrete device.
  static std::unique_ptr<gldev> make_new (display&, pixel_format, ds_format,
					  int multisample);
};

#endif // includeguard_gldev_hpp_includeguard
