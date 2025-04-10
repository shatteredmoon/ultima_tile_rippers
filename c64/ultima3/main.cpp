// Extracts Ultima III tile and text data from Commodore 64 sources.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

#define NUM_TILES       64

#define TILE_WIDTH      16
#define TILE_HALF_WIDTH 8
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

  int32_t c64ColorPalette[16] =
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
  infile.open("ultima3a.d64", std::ios::binary);

  if( infile.is_open() )
  {
    // Tile colors offset
    infile.seekg( 0xdd61, std::ios::beg );

    int32_t tileColors[NUM_TILES];
    for( int32_t i = 0; i < NUM_TILES; ++i )
    {
      tileColors[i] = infile.get();
    }

    // Tile data offset
    infile.seekg( 0x8800, std::ios::beg );

    int32_t posX{ 0 };
    int32_t posY{ 0 };

    // Tiles are set up like this:
    // The first 2 bytes represent the left-half and right half of the first tile, then the next 2-bytes are for
    // the very top of the second tile This continues until the first row of all tiles are read. Each byte = 8
    // pixels. 0 = not drawn, 1 = drawn.
    for( int32_t k = 0; k < TILE_HEIGHT; ++k )
    {
      // Draw a single tile row for all tiles. Each tile row is represented by 2 bytes.
      for( int32_t j = 0; j < NUM_TILES * 2; ++j )
      {
        int32_t index{ j / 2 };
        int32_t backIndex{ tileColors[index] & 0x0f }; // Background color
        int32_t foreIndex{ tileColors[index] & 0xf0 }; // Foreground color
        foreIndex = foreIndex >> 4;

        int32_t val{ infile.get() };

        // Draw a single tile row
        for( int32_t i = 0 ; i < TILE_HALF_WIDTH; ++i )
        {
          // We are only interested in the most significant bit per pass
          const int32_t color{ val & 0x80 ? c64ColorPalette[foreIndex] : c64ColorPalette[backIndex] };
          putpixel( backBuffer, posX, posY, color );

          // Shift into most significant bit, keeping val to byte-size
          val = ( val << 1 );
          val &= 0xff;
          ++posX;
        }
      }

      ++posY;
      posX = 0;
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

  return 0;
}
