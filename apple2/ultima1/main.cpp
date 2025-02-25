
// This program requires the OUT.SHAPES, SPA.SHAPES, TWN.CAS.SHAPES, AND ULTSHAPES files from the original .dsk image
// Additionally, MAPCHARS is required from the enhanced/re-released .dsk image
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
// MAPCHARS is 1024 bytes

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

#define CHAR_WIDTH    7
#define CHAR_HEIGHT   8

#define CHARS_PER_COL_MAPCHARS 1
#define CHARS_PER_ROW_MAPCHARS 128

#define BYTES_PER_CHAR     8
#define CHAR_BYTES_PER_ROW 128

#define MAPCHARS_BYTES         ( BYTES_PER_CHAR * CHARS_PER_ROW_MAPCHARS )
#define MAPCHARS_BUFFER_WIDTH  ( CHAR_WIDTH * CHARS_PER_ROW_MAPCHARS )
#define MAPCHARS_BUFFER_HEIGHT ( CHAR_HEIGHT * CHARS_PER_COL_MAPCHARS )


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
  // Process ULTSHAPES
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
    const char tileData{ static_cast<char>( infile.get() ) };
    rowData[currentBytes++] = ( tileData << 8 ) & 0xffff;
  }

  currentBytes = 0;

  // The next 256 bytes contain the right side of each tile
  while( currentBytes < numBytesToRead )
  {
    const char tileData{ static_cast<char>( infile.get() ) };
    rowData[currentBytes++] |= ( static_cast<uint32_t>( tileData ) & 0xff );
  }

  int32_t x{ 0 };
  int32_t y{ 0 };

  for( int32_t i = 0; i < ULTSHAPES_ROWS; ++i )
  {
    uint32_t tileData{ rowData[i] };

    const int32_t colorGroup1{ ( tileData >> 15 ) & 0x1 };
    const int32_t colorGroup2{ ( tileData >> 7 ) & 0x1 };

    // Combine both bytes into one word, ignoring the colorGroup bits
    tileData = ( ( ( tileData >> 8 ) & 0x7f ) << 7 ) | ( tileData & 0x7f );

    // Place the row of pixels
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 0  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 1  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 2  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 3  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 4  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 5  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 6  ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 7  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 8  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 9  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 10 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 11 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 12 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 13 ) & 0x3 ), colorGroup2 == 0 );

    // Next pixel row
    x = 0;
    ++y;
  }

  infile.close();

  save_pcx( "ultshapes.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  // ---------------------
  // Process MAPCHARS
  // ---------------------

  backBuffer = create_bitmap( MAPCHARS_BUFFER_WIDTH, MAPCHARS_BUFFER_HEIGHT );

  infile.open( "MAPCHARS", std::ios::in | std::ios::binary );

  if( !infile.is_open() )
  {
    return -1;
  }

  numBytesToRead = CHARS_PER_ROW_MAPCHARS * BYTES_PER_CHAR;

  x = 0;
  y = 0;

  currentBytes = 0;

  while( currentBytes < numBytesToRead )
  {
    char tileData{ static_cast<char>( infile.get() ) };

    // Place the first 7 pixels
    const int32_t colorGroup{ ( tileData >> 7 ) & 0x1 };

    // Remove the colorGroup bit
    tileData &= 0x7f;

    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 0 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 1 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 2 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 3 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 4 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 5 ) & 0x3 ), colorGroup == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 6 ) & 0x3 ), colorGroup == 0 );

    if( MAPCHARS_BUFFER_WIDTH == x )
    {
      // Wrap to the next line
      x = 0;
      ++y;
    }

    ++currentBytes;
  }

  infile.close();

  save_pcx( "mapchars.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  return 0;
}
