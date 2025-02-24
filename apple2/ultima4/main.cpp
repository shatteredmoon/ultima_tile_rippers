
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

#define TILE_WIDTH    14
#define TILE_HEIGHT   16
#define NUM_TILES     256
#define TILES_PER_COL 16
#define TILES_PER_ROW 16

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL )

#define CHAR_WIDTH    7
#define CHAR_HEIGHT   8
#define NUM_CHARS     128
#define CHARS_PER_COL 8
#define CHARS_PER_ROW 16

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
    allegro_message("Allegro initialization failed");
    return -1;
  }

  int32_t depth{ 0 };
  if( ( depth = desktop_color_depth() ) != 0 )
  {
    depth = 16;
  }

  set_color_depth( depth );
  if( set_gfx_mode( GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0 ) != 0 )
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

  std::ifstream infile1;
  infile1.open( "SHP0", std::ios::in | std::ios::binary | std::ios::ate );

  std::ifstream infile2;
  infile2.open( "SHP1", std::ios::in | std::ios::binary );

  if( !( infile1.is_open() && infile2.is_open() ) )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  int32_t numBytes{ static_cast<int32_t>( infile1.tellg() ) };

  // Reset to the beginning
  infile1.seekg( 0, std::ios::beg );

  int32_t x{ 0 };
  int32_t y{ 0 };

  int32_t currentBytes{ 0 };
  int32_t lineNum{ 0 };

  while( currentBytes < numBytes )
  {
    char tileData1{ static_cast<char>( infile1.get() ) };
    char tileData2{ static_cast<char>( infile2.get() ) };

    // Combine both bytes into one word, ignoring the colorGroup bits
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

    // Place the next 7 pixels
    const int32_t colorGroup2{ ( tileData2 >> 7 ) & 0x1 };
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 7 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 8 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 9 )  & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 10 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 11 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 12 ) & 0x3 ), colorGroup2 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 13 ) & 0x3 ), colorGroup2 == 0 );

    if( x >= TILE_BUFFER_WIDTH )
    {
      // Wrap to the next sprite
      x = 0;
      y += TILE_HEIGHT;
    }

    if( y >= TILE_BUFFER_HEIGHT )
    {
      // Start back at the top, but on the next unread line of the sprite
      y = ++lineNum;
    }

    ++currentBytes;
  }

  infile1.close();
  infile2.close();

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

  std::ifstream infile;
  infile.open( "HTXT", std::ios::in | std::ios::binary | std::ios::ate );

  if( !infile.is_open() )
  {
    return -1;
  }

  // Get file size (note that the first file was opened with ios::ate so the size can be fetched)
  numBytes = static_cast<int32_t>( infile.tellg() );

  // Reset to the beginning
  infile.seekg( 0, std::ios::beg );

  x = 0;
  y = 0;

  currentBytes = 0;
  lineNum = 0;

  while( currentBytes < numBytes )
  {
    char tileData{ static_cast<char>( infile.get() ) };

    const int32_t colorGroup1{ ( tileData >> 7 ) & 0x1 };

    // Remove the color group bit
    tileData &= 0x7f;

    // Place the 7 pixels
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 0 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 1 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 2 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 3 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 4 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 5 ) & 0x3 ), colorGroup1 == 0 );
    WriteColorForByte( backBuffer, x++, y, ( ( tileData >> 6 ) & 0x3 ), colorGroup1 == 0 );

    if( x >= CHAR_BUFFER_WIDTH )
    {
      // Wrap to the next sprite
      x = 0;
      y += CHAR_HEIGHT;
    }

    if( y >= CHAR_BUFFER_HEIGHT )
    {
      // Start back at the top, but on the next unread line of the sprite
      y = ++lineNum;
    }

    ++currentBytes;
  }

  infile.close();

  // Optionally create a vertical strip
#if EXPORT_VERTICAL_STRIP
  sourceRow = 0;
  sourceCol = 0;

  backBuffer2 = create_bitmap( CHAR_WIDTH, CHAR_HEIGHT * NUM_CHARS );
  for( int32_t i{ 0 }; i < NUM_CHARS; ++i )
  {
    blit( backBuffer, backBuffer2, sourceCol, sourceRow, 0, i * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT );
    sourceCol += CHAR_WIDTH;

    if( sourceCol >= CHAR_BUFFER_WIDTH )
    {
      sourceCol = 0;
      sourceRow += CHAR_HEIGHT;
    }
  }

  save_pcx( "text.pcx", backBuffer2, nullptr );

  destroy_bitmap( backBuffer2 );
#else
  save_pcx( "text.pcx", backBuffer, nullptr );
#endif // EXPORT_VERTICAL_STRIP

  destroy_bitmap( backBuffer );

  return 0;
}
