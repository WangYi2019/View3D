
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include "bmp_loader.hpp"

#include "utils/byte_order.hpp"

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;
typedef uint8_t BYTE;

#pragma pack(1)
struct BITMAPFILEHEADER
{
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
};
#pragma pack()

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991
static_assert (offsetof (BITMAPFILEHEADER, bfSize) == 2, "");

#pragma pack(1)
struct BITMAPINFOHEADER
{
  DWORD  biSize;
  LONG   biWidth;
  LONG   biHeight;
  WORD   biPlanes;
  WORD   biBitCount;
  DWORD  biCompression;
  DWORD  biSizeImage;
  LONG   biXPelsPerMeter;
  LONG   biYPelsPerMeter;
  DWORD  biClrUsed;
  DWORD  biClrImportant;
};
#pragma pack()

enum
{
  BI_RGB = 0,
  BI_RLE8 = 1,
  BI_RLE4 = 2,
  BI_BITFIELDS = 3,
  BI_JPEG = 4,
  BI_PNG = 5,
  BI_ALPHABITFIELDS = 6,
  BI_CMYK = 11,
  BI_CMYKRLE8 = 12,
  BI_CMYKRLE4 = 13
};

#pragma pack(1)
struct RGBQUAD
{
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
};
#pragma pack()

namespace img
{

image load_bmp_image (const char* filename)
{
  try
  {
    std::fstream in (filename, std::ios::binary | std::ios::in);
    in.exceptions (std::ifstream::failbit);

    BITMAPFILEHEADER bfh;
    in.read ((char*)&bfh, sizeof (bfh));

    if (bfh.bfType != utils::native_to (utils::little_endian, 0x4D42))
    {
      std::cerr << "load_bmp_image unknown file type" << std::endl;
      return { };
    }

    std::cout << "load_bmp_image filesize = " << bfh.bfSize
	      << " offbits = " << bfh.bfOffBits
	      << std::endl;

    BITMAPINFOHEADER bih;
    in.read ((char*)&bih, sizeof (bih));

    if (bih.biSize < sizeof (bih))
    {
      std::cerr << "load_bmp_image bih.biSize NG (got "
		<< bih.biSize << " expected " << sizeof (bih) << ")" << std::endl;
      return { };
    }

    std::cout << "load_bmp_image image size = "
	      << bih.biWidth << " x " << bih.biHeight << std::endl;

    if (bih.biPlanes != 1)
    {
      std::cerr << "load_bmp_image bih.biPlanes = " << bih.biPlanes << " NG"
		<< std::endl;
      return { };
    }

    if (bih.biCompression != BI_RGB)
    {
      std::cerr << "load_bmp_image bih.biCompression = " << bih.biCompression << " NG"
		<< std::endl;
      return { };
    }

    pixel_format pf = pixel_format::invalid;

    if (bih.biBitCount == 8)
      pf = pixel_format::l_8;
    else if (bih.biBitCount == 24)
      pf = pixel_format::bgr_888;
    else if (bih.biBitCount == 32)
      pf = pixel_format::rgba_8888;

    if (pf == pixel_format::invalid)
    {
      std::cerr << "load_bmp_image bih.biBitCount = " << bih.biBitCount << " NG"
		<< std::endl;
      return { };
    }

    in.seekg (bfh.bfOffBits, std::ios_base::beg);

    const unsigned int line_size_bytes = ((bih.biBitCount * bih.biWidth + 31) / 32) * 4;
    const unsigned int line_count = std::abs (bih.biHeight);

    struct tmp_image : public image
    {
      tmp_image (pixel_format pf, unsigned int w, unsigned int h,
		 unsigned int stride)
      {
	m_size = { w, h };
	m_bytes_per_line = stride;
	m_format = pf;
	m_data = data_buffer (stride * h);
      }
    };

    tmp_image img (pf, bih.biWidth, bih.biHeight, line_size_bytes);

#if 0
    for (unsigned int y = 0; y < line_count; ++y)
    {
      auto ydst = bih.biHeight < 0
		  ? y
		  : line_count - y - 1;

      in.read ((char*)img.data_line (ydst), line_size_bytes);
    }

#else
    // read the whole image, then swap lines if necessary.
    // this is a bit faster because of batched IO.
    in.read ((char*)img.data (), line_count * line_size_bytes);

    // if necessary, swap lines.
    if (bih.biHeight >= 0)
    {
      auto&& alloc_linebuf = std::make_unique<char[]> (line_size_bytes + 128);
      auto&& linebuf = (void*)(((uintptr_t)alloc_linebuf.get () + 127) & ~uintptr_t (127));

      for (unsigned int y = 0; y < line_count / 2; ++y)
      {
	std::memcpy (linebuf, img.data_line (y), line_size_bytes);
	std::memcpy (img.data_line (y), img.data_line (line_count - y - 1), line_size_bytes);
	std::memcpy (img.data_line (line_count - y - 1), linebuf, line_size_bytes);
      }
    }
#endif

    return std::move (img);
  }
  catch (const std::exception& e)
  {
    std::cerr << "load_bmp_image exception: " << e.what () << std::endl;
    return { };
  }
}

} // namespace img