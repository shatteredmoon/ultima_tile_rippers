// Extracts Ultima II tile data from Commodore 64 sources.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

#define NUM_TILES       64
#define PIXELS_PER_QUAD 8
#define PIXELS_PER_BYTE 8

#define TILE_WIDTH      16
#define TILE_HEIGHT     16
#define TILES_PER_COL   1
#define TILES_PER_ROW   64

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * TILES_PER_ROW )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT * TILES_PER_COL )

#define EXPORT_VERTICAL_STRIP 0


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

  int32_t xOffset{ 0 };
  int32_t yOffset{ 0 };
  const int32_t bit{ 0x80 };

  std::ifstream infile;
  infile.open("shapes.prg", std::ios::binary);

  BITMAP* backBuffer{ create_bitmap( TILE_BUFFER_WIDTH, TILE_BUFFER_HEIGHT ) };

  // These are the tile color lookup values ripped from offset 0x1aa1 using ul2-1.d64 (or 0x1a6b into the ultima ii.prg
  // file). The 1st 30 tiles are straight forward lookups. I've never figured out how the rest of the values are
  // represented, but most of them are orange, apart from the timegate tile in the middle of all the signs, and then
  // the blue player characters to the far right.
  const int32_t tileColors[] =
  {
    0x0E,0x06,0x05,0x05,0x04,0x08,0x08,0x00,
    0x00,0x0B,0x08,0x0E,0x0B,0x0B,0x0B,0x0B,
    0x01,0x09,0x06,0x0B,0x0B,0x06,0x01,0x01,
    0x0B,0x0B,0x0B,0x04,0x0A,0x01,0x00,0x08,
    0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
    0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
    0x0B,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
    0x08,0x08,0x08,0x08,0x06,0x06,0x06,0x06
  };

  if( infile.is_open() )
  {
    // This code uses shapes.prg
    infile.seekg( 2, std::ios::beg );

    // --------------------------------------------
    // 1 byte = 8 pixels - image stored in quadrants top-left, bottom-left, top-right, bottom-right
    for( int32_t k = 0; k < NUM_TILES; ++k )
    {
      const int32_t tileColor{ tileColors[k] };

      // top-left
      int32_t posX = 0;
      int32_t posY = 0;

      for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
      {
        const int32_t val{ infile.get() };
        for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
        {
          const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0xd] ) };
          putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
        }

        posX = 0;
        ++posY;
      }

      // bottom left
      posX = 0;
      posY = PIXELS_PER_QUAD;

      for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j)
      {
        const int32_t val{ infile.get() };
        for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
        {
          const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0xd] ) };
          putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
        }

        posX = 0;
        ++posY;
      }

      // top right
      posX = PIXELS_PER_QUAD;
      posY = 0;

      for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j)
      {
        const int32_t val{ infile.get() };
        for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
        {
          const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0xd] ) };
          putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
        }

        posX = PIXELS_PER_QUAD;
        ++posY;
      }

      // bottom right
      posX = PIXELS_PER_QUAD;
      posY = PIXELS_PER_QUAD;

      for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j)
      {
        const int32_t val{ infile.get() };
        for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
        {
          const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0xd] ) };
          putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
        }

        posX = PIXELS_PER_QUAD;
        ++posY;
      }

      xOffset += TILE_WIDTH;
      if( xOffset > TILE_BUFFER_WIDTH )
      {
        xOffset = 0;
        yOffset += TILE_WIDTH;
      }

      if( infile.eof() )
      {
        break;
      }
    }
  }

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

  return 0;
}
