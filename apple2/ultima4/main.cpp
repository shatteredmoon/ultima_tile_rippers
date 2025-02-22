
// This program requires the SHP0 and SHP1 files extracted from the Apple II Ultima IV Boot.dsk
// Apple II disk and file archive manager: https://a2ciderpress.com/

// Resources:
// https://u4a2.com/
// https://en.wikipedia.org/wiki/Apple_II_graphics
// https://retrocomputing.stackexchange.com/questions/6271/what-determines-the-color-of-every-8th-pixel-on-the-apple-ii
// https://www.xtof.info/hires-graphics-apple-ii.html


#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

// SHP0 / SHP1
// Num images across 16
// Num images down 8
// Num images 128
// Total bytes = 4096
// Bytes per image = 32 (256 bits)
// Pixels per image = 14 x 16 = 224 pixels
// Leaving 32 bits = or 2 pixels per row
// Num images per row = 16
// Num pixels per row = 16 images * 14 pixels = 224
// Num images per column = 16
// Num pixels per column = 16 images * 16 pixels = 256

#define BUFFER_WIDTH    224
#define BUFFER_HEIGHT   256

#define SPRITE_WIDTH    14
#define SPRITE_HEIGHT   16

enum colorType
{
  Green,
  Orange,
  Violet,
  Blue,
  White,
  Black
};

int colorTable[6];


void WriteColorForByte( BITMAP* backBuffer, int32_t x, int32_t y, char value, bool useGreenViolet )
{
  colorType color{ Black };

  switch( value )
  {
    case 0x0: color = Black; break;
    case 0x1: color = useGreenViolet ? Green : Orange; break;
    case 0x2: color = useGreenViolet ? Violet : Blue; break;
    case 0x3: color = White; break;
  }

  putpixel( backBuffer, x, y, colorTable[color] );
}


int32_t main()
{
  if( allegro_init() != 0 )
  {
    allegro_message("Allegro initialization failed");
    return -1;
  }

  int32_t depth{ 0 };
  if( ( depth = desktop_color_depth() ) != 0 )
  {
    depth = 16;
  }

  set_color_depth( depth );
  if( set_gfx_mode( GFX_AUTODETECT_WINDOWED, BUFFER_WIDTH, BUFFER_HEIGHT, 0, 0 ) != 0 )
  {
    allegro_message( "Failed to set graphics mode" );
    return -1;
  }

  colorTable[Green]  = makecol( 0x25, 0xBE, 0x00 );
  colorTable[Orange] = makecol( 0xE5, 0x50, 0x00 );
  colorTable[Violet] = makecol( 0x9E, 0x00, 0xFF );
  colorTable[Blue]   = makecol( 0x00, 0x7E, 0xFF );
  colorTable[White]  = makecol( 0xFF, 0xFF, 0xFF );
  colorTable[Black]  = makecol( 0x00, 0x00, 0x00 );

  BITMAP* pcBackBuffer{ create_bitmap( BUFFER_WIDTH, BUFFER_HEIGHT ) };

  std::ifstream infile1;
  infile1.open( "SHP0", std::ios::in | std::ios::binary | std::ios::ate );

  std::ifstream infile2;
  infile2.open( "SHP1", std::ios::in | std::ios::binary );

  if( !( infile1.is_open() && infile2.is_open() ) )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  const auto size{ infile1.tellg() };
  int32_t numBytes = (int32_t)size;

  // Reset to the beginning
  infile1.seekg( 0, std::ios::beg );

  int32_t x{ 0 };
  int32_t y{ 0 };

  int32_t currentBytes{ 0 };
  int32_t lineNum = 0;

  while( currentBytes < numBytes )
  {
    char pixel1{ static_cast<char>( infile1.get() ) };
    char pixel2{ static_cast<char>( infile2.get() ) };

    // Combine both pixels into one word, ignoring the colorGroup bits
    int32_t pixel = ( ( pixel2 & 0x7f ) << 7 ) | ( pixel1 & 0x7f );

    // Place the first 7 pixels
    const int32_t colorGroup1{ ( pixel1 >> 7 ) & 0x1 };
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 0 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 1 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 2 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 3 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 4 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 5 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 6 ) & 0x3 ), colorGroup1 == 0 );

    // Place the next 7 pixels
    const int32_t colorGroup2{ ( pixel2 >> 7 ) & 0x1 };
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 7 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 8 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 9 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 10 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 11 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 12 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( pcBackBuffer, x++, y, ( ( pixel >> 13 ) & 0x3 ), colorGroup2 == 0 );

    if( x % BUFFER_WIDTH == 0 )
    {
      // Wrap to the next sprite
      x = 0;
      y += SPRITE_HEIGHT;
    }

    if( y >= BUFFER_HEIGHT )
    {
      // Start back at the top, but on the next unread line of the sprite
      y = ++lineNum;
    }

    ++currentBytes;
  }

  infile1.close();
  infile2.close();

  blit( pcBackBuffer, screen, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT );

  save_pcx( "ultima4.pcx", pcBackBuffer, nullptr );

  return 0;
}
