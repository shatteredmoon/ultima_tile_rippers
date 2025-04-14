// Extracts Ultima IV tile data from Commodore 64 sources.

// The tiles are stored in file 41, which loads at $b000 - $cfff. They are stored in two halves, with the upper half of
// each tile at $b000 - $bfff, and the lower half in $c000 - $cfff. The tiles are in normal C64 bitmap format( 1bpp,
// 8 bytes per cell, 40 cells per line ). Color data is in file 4c, at $e1b0 - $e2af.

// The first half of the tiles are at 0x16700 and the second are at 0x17700. The colours are at 0x130b0.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

#define NUM_TILES       256
#define PIXELS_PER_QUAD 8
#define PIXELS_PER_BYTE 8

#define TILE_WIDTH      16
#define TILE_HEIGHT     16
#define TILES_PER_COL   1
#define TILES_PER_ROW   256

#define QUADS_PER_ROW   512
#define QUADS_PER_COL   2

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL )

#define CHAR_HEIGHT     8
#define CHAR_WIDTH      8

#define NUM_MOON_TILES  8

#define EXPORT_VERTICAL_STRIP 0


int32_t main()
{
  if( allegro_init() != 0 )
  {
    allegro_message( "Allegro initialization failed" );
    return -1;
  }

  int32_t depth{ desktop_color_depth() };
  set_color_depth( depth );

  if( set_gfx_mode( GFX_AUTODETECT_WINDOWED, 1024, 768, 0, 0 ) != 0 )
  {
    allegro_message( "Failed to set graphics mode" );
    return -1;
  }

  int32_t c64Colors[] =
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

  BITMAP* backBuffer{ create_bitmap( TILE_BUFFER_WIDTH, TILE_BUFFER_HEIGHT ) };

  std::ifstream infile;

  infile.open( "colors.prg", std::ios::binary );
  if( !infile.is_open() )
  {
    return -1;
  }

  // The tile colors exist in the .d64 image at offset 0x130b0.
  int32_t tileColors[NUM_TILES];
  for( int32_t i = 0; i < NUM_TILES; ++i )
  {
    tileColors[i] = ( infile.get() >> 4 );
  }

  infile.close();

  infile.open( "shapes.prg", std::ios::binary );
  if( !infile.is_open() )
  {
    return -1;
  }

  const int32_t bit{ 0x80 };

  // Tile/shape data exists in the .d64 image at offset 0x16700.
  // Tiles are set up like this: The first 2 bytes represent the left-half and right half of the first tile, then the
  // next 2-bytes are for the very top of the second tile This continues until the first row of all tiles are read.
  // Each byte = 8 pixels.
  for( int32_t l = 0; l < QUADS_PER_COL; ++l )
  {
    for( int32_t k = 0; k < QUADS_PER_ROW; ++k )
    {
      const int32_t tileNum{ k / 2 };
      const int32_t tileColor{ tileColors[tileNum] };

      for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
      {
        const int32_t val{ infile.get() };
        for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
        {
          const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0] ) };
          putpixel( backBuffer, k * PIXELS_PER_QUAD + i, l * PIXELS_PER_QUAD + j, color );
        }
      }
    }
  }

  infile.close();

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

  backBuffer = create_bitmap( CHAR_WIDTH * NUM_MOON_TILES, CHAR_HEIGHT );

  infile.open( "moons.prg", std::ios::binary );
  if( !infile.is_open() )
  {
    return -1;
  }

  // Moon data exists in the .d64 image at offset 0x16109.
  for( int32_t k = 0; k < NUM_MOON_TILES; ++k )
  {
    for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
    {
      const int32_t val{ infile.get() };
      for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
      {
        const int32_t color{ ( val & ( bit >> i ) ? c64Colors[1] : c64Colors[0] ) };
        putpixel( backBuffer, k * PIXELS_PER_QUAD + i, j, color );
      }
    }
  }

  // Optionally create a vertical strip
#if EXPORT_VERTICAL_STRIP
  sourceRow = 0;
  sourceCol = 0;

  backBuffer2  = create_bitmap( CHAR_WIDTH, CHAR_HEIGHT * NUM_MOON_TILES );

  for( uint32_t i{ 0 }; i < NUM_MOON_TILES; ++i )
  {
    blit( backBuffer, backBuffer2, sourceCol, sourceRow, 0, i * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT );
    sourceCol += CHAR_WIDTH;

    if( sourceCol >= ( CHAR_WIDTH * NUM_MOON_TILES ) )
    {
      sourceCol = 0;
      sourceRow += CHAR_HEIGHT;
    }
  }

  save_pcx( "moons.pcx", backBuffer2, nullptr );

  destroy_bitmap( backBuffer2 );
#else
  save_pcx( "moons.pcx", backBuffer, nullptr );
#endif // EXPORT_VERTICAL_STRIP

  return 0;
}
