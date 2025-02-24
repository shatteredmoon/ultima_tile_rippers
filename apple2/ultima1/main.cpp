
// This program requires the OUT.SHAPES, SPA.SHAPES, TWN.CAS.SHAPES, AND ULTSHAPES files from the .dsk image
// Apple II disk and file archive manager: https://a2ciderpress.com/

// Resources:
// https://u4a2.com/
// https://en.wikipedia.org/wiki/Apple_II_graphics
// https://retrocomputing.stackexchange.com/questions/6271/what-determines-the-color-of-every-8th-pixel-on-the-apple-ii
// https://www.xtof.info/hires-graphics-apple-ii.html
// https://groups.google.com/g/comp.sys.apple2/c/2NHj_6azS_g/m/H67Cijk7ViEJ

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

// OUT.SHAPES size 512 bytes
// SPA.SHAPES size 860 bytes
// TWN.CAS.SHAPES size 256 bytes
// ULTSHAPES size 763 bytes - possibly only contains 512 bytes of tile data?

#define TILE_WIDTH    14
#define TILE_HEIGHT   16

#define TILES_PER_COL_ULTSHAPES 16
#define TILES_PER_ROW_ULTSHAPES 1

#define BYTES_PER_TILE     32
#define TILE_BYTES_PER_ROW 2

#define ULTSHAPES_BYTES         ( BYTES_PER_TILE * TILES_PER_COL_ULTSHAPES )
#define ULTSHAPES_ROWS          ( ULTSHAPES_BYTES / TILE_BYTES_PER_ROW )
#define ULTSHAPES_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW_ULTSHAPES )
#define ULTSHAPES_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL_ULTSHAPES )


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

  colorTable[Green]  = makecol( 0x25, 0xBE, 0x00 );
  colorTable[Orange] = makecol( 0xE5, 0x50, 0x00 );
  colorTable[Violet] = makecol( 0x9E, 0x00, 0xFF );
  colorTable[Blue]   = makecol( 0x00, 0x7E, 0xFF );
  colorTable[White]  = makecol( 0xFF, 0xFF, 0xFF );
  colorTable[Black]  = makecol( 0x00, 0x00, 0x00 );

  // ---------------------
  // Process tile graphics
  // ---------------------

  BITMAP* backBuffer{ create_bitmap( ULTSHAPES_BUFFER_WIDTH, ULTSHAPES_BUFFER_HEIGHT ) };

  std::ifstream infile;
  infile.open( "ULTSHAPES", std::ios::in | std::ios::binary );

  if( !infile.is_open() )
  {
    return -1;
  }

  int32_t numBytesToRead{ ULTSHAPES_BYTES / 2 };
  int32_t currentBytes{ 0 };
  uint32_t rowData[ULTSHAPES_ROWS];

  // The first 256 bytes contain the left side of each tile
  while( currentBytes < numBytesToRead )
  {
    const char pixel{ static_cast<char>( infile.get() ) };
    rowData[currentBytes++] = ( pixel << 8 ) & 0xffff;
  }

  currentBytes = 0;

  // The next 256 bytes contain the right side of each tile
  while( currentBytes < numBytesToRead )
  {
    const char pixel{ static_cast<char>( infile.get() ) };
    rowData[currentBytes++] |= ( static_cast<uint32_t>( pixel ) & 0xff );
  }

  int32_t x{ 0 };
  int32_t y{ 0 };

  for( int32_t i = 0; i < ULTSHAPES_ROWS; ++i )
  {
    uint32_t data{ rowData[i] };

    const int32_t colorGroup1{ ( data >> 15 ) & 0x1 };
    const int32_t colorGroup2{ ( data >> 7 ) & 0x1 };

    // Combine both pixels into one word, ignoring the colorGroup bits
    data = ( ( ( data >> 8 ) & 0x7f ) << 7 ) | ( data & 0x7f );

    // Place the row of pixels
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 0  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 1  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 2  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 3  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 4  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 5  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 6  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 7  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 8  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 9  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 10 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 11 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 12 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( data >> 13 ) & 0x3 ), colorGroup2 == 0 );

    // Next pixel row
    x = 0;
    ++y;
  }

  infile.close();

  save_pcx( "ultshapes.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  return 0;
}
