// Extracts Ultima IV tile and text data from Apple ][ sources.
// This program requires the SHP0 and SHP1 files extracted from the Apple II Ultima IV Boot.dsk
// Apple II disk and file archive manager: https://a2ciderpress.com/

// Resources:
// https://u4a2.com/
// https://en.wikipedia.org/wiki/Apple_II_graphics
// https://retrocomputing.stackexchange.com/questions/6271/what-determines-the-color-of-every-8th-pixel-on-the-apple-ii
// https://www.xtof.info/hires-graphics-apple-ii.html
// Gil Megidish's pixel rendering algorithm

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
  uint32_t numBytes{ static_cast<uint32_t>( infile1.tellg() ) };

  // Reset to the beginning
  infile1.seekg( 0, std::ios::beg );

  uint32_t x{ 0 };
  uint32_t y{ 0 };

  uint32_t currentBytes{ 0 };
  uint32_t lineNum{ 0 };

  while( currentBytes < numBytes )
  {
    const uint8_t tileData1{ static_cast<uint8_t>( infile1.get() ) };
    const uint8_t tileData2{ static_cast<uint8_t>( infile2.get() ) };

    // Combine both bytes into one word, ignoring the colorGroup bits
    const uint16_t tileData{ static_cast<uint16_t>( ( ( tileData2 & 0x7f ) << 7 ) | ( tileData1 & 0x7f ) ) };

    // Place the first 7 pixels
    const bool colorGroup1{ ( ( tileData1 >> 7 ) & 0x1 ) == 0 };
    Draw( backBuffer, x++, y, ( tileData >> 0 ) & 0x3, false, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 1 ) & 0x3, ( tileData >> 0 ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 2 ) & 0x3, ( tileData >> 1 ) & 0x1, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 3 ) & 0x3, ( tileData >> 2 ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 4 ) & 0x3, ( tileData >> 3 ) & 0x1, colorGroup1, true );
    Draw( backBuffer, x++, y, ( tileData >> 5 ) & 0x3, ( tileData >> 4 ) & 0x1, colorGroup1, false );
    Draw( backBuffer, x++, y, ( tileData >> 6 ) & 0x3, ( tileData >> 5 ) & 0x1, colorGroup1, true );

    // Place the next 7 pixels
    const bool colorGroup2{ ( ( tileData2 >> 7 ) & 0x1 ) == 0 };
    Draw( backBuffer, x++, y, ( tileData >> 7 )  & 0x3, ( tileData >> 6 )  & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 8 )  & 0x3, ( tileData >> 7 )  & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 9 )  & 0x3, ( tileData >> 8 )  & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 10 ) & 0x3, ( tileData >> 9 )  & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 11 ) & 0x3, ( tileData >> 10 ) & 0x1, colorGroup2, false );
    Draw( backBuffer, x++, y, ( tileData >> 12 ) & 0x3, ( tileData >> 11 ) & 0x1, colorGroup2, true );
    Draw( backBuffer, x++, y, ( tileData >> 13 ) & 0x3, ( tileData >> 12 ) & 0x1, colorGroup2, false );

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
  uint32_t sourceRow{ 0 };
  uint32_t sourceCol{ 0 };

  BITMAP* backBuffer2{ create_bitmap( TILE_WIDTH, TILE_HEIGHT * NUM_TILES ) };
  for( uint32_t i{ 0 }; i < NUM_TILES; ++i )
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
    uint8_t tileData{ static_cast<uint8_t>( infile.get() ) };

    const bool colorGroup{ ( ( tileData >> 7 ) & 0x1 ) == 0 };

    // Remove the color group bit
    tileData &= 0x7f;

    // Place the 7 pixels
    Draw( backBuffer, x++, y, ( ( tileData >> 0 ) & 0x3 ), false, colorGroup, true );
    Draw( backBuffer, x++, y, ( ( tileData >> 1 ) & 0x3 ), ( tileData >> 0 ) & 0x1, colorGroup, false );
    Draw( backBuffer, x++, y, ( ( tileData >> 2 ) & 0x3 ), ( tileData >> 1 ) & 0x1, colorGroup, true );
    Draw( backBuffer, x++, y, ( ( tileData >> 3 ) & 0x3 ), ( tileData >> 2 ) & 0x1, colorGroup, false );
    Draw( backBuffer, x++, y, ( ( tileData >> 4 ) & 0x3 ), ( tileData >> 3 ) & 0x1, colorGroup, true );
    Draw( backBuffer, x++, y, ( ( tileData >> 5 ) & 0x3 ), ( tileData >> 4 ) & 0x1, colorGroup, false );
    Draw( backBuffer, x++, y, ( ( tileData >> 6 ) & 0x3 ), ( tileData >> 5 ) & 0x1, colorGroup, true );

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
