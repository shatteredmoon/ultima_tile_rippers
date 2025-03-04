
// This program requires the SHAPES AND HTXT files from the .dsk image
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

#define TILE_WIDTH    14
#define TILE_HEIGHT   16
#define NUM_TILES     64
#define TILES_PER_COL 1
#define TILES_PER_ROW 64

#define TILE_BYTES_PER_ROW 2

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL )

#define CHAR_WIDTH    7
#define CHAR_HEIGHT   8
#define NUM_CHARS     256
#define CHARS_PER_COL 256
#define CHARS_PER_ROW 1

#define CHAR_BYTES_PER_ROW 1

#define CHAR_BUFFER_WIDTH  ( CHAR_WIDTH * CHARS_PER_ROW )
#define CHAR_BUFFER_HEIGHT ( CHAR_HEIGHT * CHARS_PER_COL )

#define EXPORT_VERTICAL_STRIP 0


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

  BITMAP* backBuffer{ create_bitmap( TILE_BUFFER_WIDTH, TILE_BUFFER_HEIGHT ) };

  std::ifstream infile;
  infile.open( "SHAPES", std::ios::in | std::ios::binary );

  if( !infile.is_open() )
  {
    return -1;
  }

  int32_t numBytesToRead{ NUM_TILES * TILE_HEIGHT * TILE_BYTES_PER_ROW };

  int32_t x{ 0 };
  int32_t y{ 0 };

  int32_t currentBytes{ 0 };
  int32_t lineNum{ 0 };

  while( currentBytes < numBytesToRead )
  {
    char tileData1{ static_cast<char>( infile.get() ) };
    char tileData2{ static_cast<char>( infile.get() ) };

    int32_t tileData{ ( ( tileData2 & 0x7f ) << 7 ) | ( tileData1 & 0x7f ) };

    // Place the first 7 pixels
    const int32_t colorGroup1{ ( tileData1 >> 7 ) & 0x1 };
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 0 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 1 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 2 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 3 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 4 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 5 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 6 ) & 0x3 ), colorGroup1 == 0 );

    const int32_t colorGroup2{ ( tileData2 >> 7 ) & 0x1 };
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 7  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 8  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 9  ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 10 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 11 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 12 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 13 ) & 0x3 ), colorGroup2 == 0 );

    if( x >= TILE_BUFFER_WIDTH )
    {
      // Wrap to next line
      x = 0;
      y += 1;
    }

    currentBytes += TILE_BYTES_PER_ROW;
  }

  infile.close();

  // Optionally create a vertical strip
#if EXPORT_VERTICAL_STRIP
  int32_t sourceRow{ 0 };
  int32_t sourceCol{ 0 };

  BITMAP* backBuffer2{ create_bitmap( TILE_WIDTH, TILE_HEIGHT * NUM_TILES ) };
  for( int32_t i{ 0 }; i < NUM_TILES; ++i )
  {
    blit( backBuffer, backBuffer2, sourceCol, sourceRow, 0, i * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT );
    sourceCol += TILE_WIDTH;

    if( sourceCol >= TILE_BUFFER_WIDTH )
    {
      sourceCol = 0;
      sourceRow += TILE_HEIGHT;
    }
  }

  save_pcx( "tiles.pcx", backBuffer2, nullptr );

  destroy_bitmap( backBuffer2 );
#else
  save_pcx( "tiles.pcx", backBuffer, nullptr );
#endif // EXPORT_VERTICAL_STRIP

  destroy_bitmap( backBuffer );

  // ---------------------
  // Process text graphics
  // ---------------------

  backBuffer = create_bitmap( CHAR_BUFFER_WIDTH, CHAR_BUFFER_HEIGHT );

  infile.open( "HTXT", std::ios::in | std::ios::binary );

  if( !infile.is_open() )
  {
    return -1;
  }

  numBytesToRead = NUM_CHARS * CHAR_HEIGHT * TILE_BYTES_PER_ROW;

  x = 0;
  y = 0;

  currentBytes = 0;
  lineNum = 0;

  while( currentBytes < numBytesToRead )
  {
    char tileData{ static_cast<char>( infile.get() ) };

    // Place the 7 pixels
    const int32_t colorGroup1{ ( tileData >> 7 ) & 0x1 };

    // Remove the color group bit
    tileData &= 0x7f;

    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 0 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 1 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 2 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 3 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 4 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 5 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 6 ) & 0x3 ), colorGroup1 == 0 );

    // Wrap to next line
    x = 0;
    ++y;

    currentBytes += CHAR_BYTES_PER_ROW;
  }

  infile.close();

  // Exported as a vertical strip by default
  save_pcx( "text.pcx", backBuffer, nullptr );

  destroy_bitmap( backBuffer );

  return 0;
}
