// Extracts Ultima I tile and text data from Apple ][ sources.
// This program requires the OUT.SHAPES, SPA.SHAPES, TWN.CAS.SHAPES, AND ULTSHAPES files from the original .dsk image
// Additionally, MAPCHARS is required from the enhanced/re-released .dsk image
// Apple II disk and file archive manager: https://a2ciderpress.com/

// Resources:
// https://u4a2.com/
// https://en.wikipedia.org/wiki/Apple_II_graphics
// https://retrocomputing.stackexchange.com/questions/6271/what-determines-the-color-of-every-8th-pixel-on-the-apple-ii
// https://www.xtof.info/hires-graphics-apple-ii.html
// https://groups.google.com/g/comp.sys.apple2/c/2NHj_6azS_g/m/H67Cijk7ViEJ
// Gil Megidish's pixel rendering algorithm

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


// From Gil Megidish:
// The algorithm is this: for any given pixel at x, if x is 1 and any of( x - 1 ), ( x + 1 ) are 1s, then pixel at x is
// white. if x is 0 and the two adjacent are also zero, then pixel at x is black. Now it's tricky. If x is 1 and both
// adjacent pixels are 0, then pixel at x is green/purple (if odd or even), there's also blue / orange for second
// palette. If x is 0, and both adjacent are 1, then the previous algorithm also catches. Sum it up, color at pixel x
// depends on the two adjacent pixels, the MSB of the byte being rendered, and if this pixel is odd / even.

void Draw( BITMAP* buffer, uint32_t x, uint32_t y, uint8_t value, bool lastBitOn, bool firstColorGroup, bool odd )
{
  colorType color{ Black };

  if( ( value & 0x1 ) == 1 )
  {
    if( lastBitOn || ( ( value >> 1 ) & 0x1 ) )
    {
      color = White;
    }
    else if( !lastBitOn && ( ( ( value >> 1 ) & 0x1 ) == 0 ) )
    {
      if( odd )
      {
        color = firstColorGroup ? Green : Orange;
      }
      else
      {
        color = firstColorGroup ? Violet : Blue;
      }
    }
  }
  else
  {
    if( !lastBitOn && ( ( ( value >> 1 ) & 0x1 ) == 0 ) )
    {
      color = Black;
    }
    if( lastBitOn && ( ( value >> 1 ) & 0x1 ) )
    {
      if( odd )
      {
        color = firstColorGroup ? Violet : Blue;
      }
      else
      {
        color = firstColorGroup ? Green : Orange;
      }
    }
  }

  putpixel( buffer, x, y, colorTable[color] );
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

  uint32_t numBytesToRead{ ULTSHAPES_BYTES / 2 };
  uint32_t currentBytes{ 0 };
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

  uint32_t x{ 0 };
  uint32_t y{ 0 };

  for( uint32_t i = 0; i < ULTSHAPES_ROWS; ++i )
  {
    uint32_t tileData{ rowData[i] };

    const bool colorGroup1{ ( ( tileData >> 15 ) & 0x1 ) == 0 };
    const bool colorGroup2{ ( ( tileData >> 7 ) & 0x1 ) == 0 };

    // Combine both bytes into one word, ignoring the colorGroup bits
    tileData = ( ( ( tileData >> 8 ) & 0x7f ) << 7 ) | ( tileData & 0x7f );

    // Place the row of pixels
    Draw( backBuffer, x++, y, ( tileData >> 0  ) & 0x3, false, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 1  ) & 0x3, ( tileData >> 0  ) & 0x1, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 2  ) & 0x3, ( tileData >> 1  ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 3  ) & 0x3, ( tileData >> 2  ) & 0x1, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 4  ) & 0x3, ( tileData >> 3  ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 5  ) & 0x3, ( tileData >> 4  ) & 0x1, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 6  ) & 0x3, ( tileData >> 5  ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 7  ) & 0x3, ( tileData >> 6  ) & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 8  ) & 0x3, ( tileData >> 7  ) & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 9  ) & 0x3, ( tileData >> 8  ) & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 10 ) & 0x3, ( tileData >> 9  ) & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 11 ) & 0x3, ( tileData >> 10 ) & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 12 ) & 0x3, ( tileData >> 11 ) & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 13 ) & 0x3, ( tileData >> 12 ) & 0x1, colorGroup2, true );

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
    uint8_t tileData{ static_cast<uint8_t>( infile.get() ) };

    // Place the first 7 pixels
    const bool colorGroup{ ( ( tileData >> 7 ) & 0x1 ) == 0 };

    // Remove the colorGroup bit
    tileData &= 0x7f;

    Draw( backBuffer, x++, y, ( tileData >> 0 ) & 0x3, false, colorGroup, false );
    Draw( backBuffer, x++, y, ( tileData >> 1 ) & 0x3, ( tileData >> 0 ) & 0x1, colorGroup, true );
    Draw( backBuffer, x++, y, ( tileData >> 2 ) & 0x3, ( tileData >> 1 ) & 0x1, colorGroup, false );
    Draw( backBuffer, x++, y, ( tileData >> 3 ) & 0x3, ( tileData >> 2 ) & 0x1, colorGroup, true );
    Draw( backBuffer, x++, y, ( tileData >> 4 ) & 0x3, ( tileData >> 3 ) & 0x1, colorGroup, false );
    Draw( backBuffer, x++, y, ( tileData >> 5 ) & 0x3, ( tileData >> 4 ) & 0x1, colorGroup, true );
    Draw( backBuffer, x++, y, ( tileData >> 6 ) & 0x3, ( tileData >> 5 ) & 0x1, colorGroup, false );

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
