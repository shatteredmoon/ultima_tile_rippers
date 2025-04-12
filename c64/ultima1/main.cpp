// Extracts Ultima I tile data from Commodore 64 sources.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"


int32_t main()
{
  if( allegro_init() != 0 )
  {
    allegro_message( "Allegro initialization failed" );
    return -1;
  }

  int32_t depth{ 0 };
  if( ( depth = desktop_color_depth() ) != 0 )
  {
    depth = 16;
  }

  set_color_depth( depth );
  if( set_gfx_mode( GFX_AUTODETECT_WINDOWED, 1024, 768, 0, 0 ) != 0 )
  {
    allegro_message( "Failed to set graphics mode" );
    return -1;
  }

  int32_t c64Colors[16] =
  {
    makecol( 0x00, 0x00, 0x00 ), // Black
    makecol( 0xff, 0xff, 0xff ), // White
    makecol( 0x93, 0x3a, 0x4c ), // Red
    makecol( 0xb6, 0xfa, 0xfa ), // Cyan
    makecol( 0xd2, 0x7d, 0xed ), // Purple
    makecol( 0x6a, 0xcf, 0x6f ), // Green
    makecol( 0x4f, 0x44, 0xd8 ), // Blue
    makecol( 0xfb, 0xfb, 0x8b ), // Yellow
    makecol( 0xd8, 0x9c, 0x5b ), // Orange
    makecol( 0x7f, 0x53, 0x07 ), // Brown
    makecol( 0xef, 0x83, 0x9f ), // Light Red
    makecol( 0x57, 0x57, 0x53 ), // Dark Gray
    makecol( 0x57, 0x57, 0x53 ), // Gray
    makecol( 0xb7, 0xfb, 0xbf ), // Light Green
    makecol( 0xa3, 0x97, 0xff ), // Light Blue
    makecol( 0xa3, 0xa7, 0xa7 )  // Light Gray
  };

  BITMAP* backBuffer{ create_bitmap( 768, 16 ) };

  std::ifstream infile;
  infile.open( "st.prg", std::ios::binary );
  if( !infile.is_open() )
  {
    return -1;
  }

  int32_t val { 0 };
  int32_t posX { 0 };
  int32_t posY { 0 };
  int32_t xOffset { 0 };
  int32_t yOffset { 0 };

  // This code uses st.prg or st-tiles.prg
  infile.seekg( 2, std::ios::beg );

  // other tiles are stored in quadrants:
  const int32_t numTiles{ 48 };

  // --------------------------------------------
  // 1 byte = 8 pixels
  for( int32_t k = 0; k < numTiles; k++ )
  {
    posX = 0;
    posY = 0;

    for( int32_t i = 0; i < 8; ++i )
    {
      val = infile.get();
      int32_t c = val & 0xf;
      if( c == 0 ) c = 1; // switch to white

      // Temp
      c = 1;

      int32_t v = val & 0x80;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x40;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x20;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x10;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x8;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x4;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x2;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x1;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );

      posX = 0;
      ++posY;
    }

    posX = 8;
    posY = 0;

    for( int32_t i = 0; i < 8; ++i )
    {
      val = infile.get();
      int32_t c = val & 0xf;
      if( c == 0 ) c = 1; // switch to white

      // Temp
      c = 1;

      int32_t v = val & 0x80;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x40;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x20;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x10;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x8;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x4;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x2;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x1;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );

      posX = 8;
      ++posY;
    }

    posX = 0;
    posY = 8;

    for( int32_t i = 0; i < 8; ++i )
    {
      val = infile.get();
      int32_t c = val & 0xf;
      if( c == 0 ) c = 1; // switch to white

      // Temp
      c = 1;

      int32_t v = val & 0x80;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x40;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x20;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x10;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x8;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x4;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x2;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x1;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );

      posX = 0;
      ++posY;
    }

    posX = 8;
    posY = 8;

    for( int32_t i = 0; i < 8; ++i )
    {
      val = infile.get();
      int32_t c = val & 0xf;
      if( c == 0 ) c = 1; // switch to white

      // Temp
      c = 1;

      int32_t v = val & 0x80;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x40;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x20;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x10;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x8;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x4;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x2;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );
      v = val & 0x1;
      putpixel( backBuffer, xOffset + posX++, yOffset + posY, ( v ? c64Colors[c] : c64Colors[0] ) );

      posX = 8;
      ++posY;
    }

    xOffset += 16;
    if( xOffset >= 768 )
    {
      xOffset = 0;
      yOffset += 16;
    }

    if( infile.eof() )
    {
      break;
    }
  }

  save_pcx( "tiles.pcx", backBuffer, nullptr );
  destroy_bitmap( backBuffer );

  return 0;
}
