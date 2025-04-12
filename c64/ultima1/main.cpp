// Extracts Ultima I tile data from Commodore 64 sources.

#define ALLEGRO_NO_MAGIC_MAIN
#define ALLEGRO_STATICLINK 1

#include <fstream>

#include "../../allegro/include/allegro.h"
#include "../../allegro/include/winalleg.h"

#define NUM_TILES       48
#define PIXELS_PER_QUAD 8
#define PIXELS_PER_BYTE 8

#define TILE_HEIGHT     16
#define TILE_WIDTH      16

#define TILE_BUFFER_WIDTH  ( TILE_WIDTH * NUM_TILES )
#define TILE_BUFFER_HEIGHT ( TILE_HEIGHT )

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

  const int32_t c64Colors[16] =
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

  // TODO: Should be read from disk
  const int32_t tileColors[] =
  {
    0x6,0x5,0x5,0x1,0x1,0x1,0x1,0x1,
    0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,
    0x1,0x1,0x5,0x5,0x1,0x1,0x1,0x1,
    0x1,0x1,0x1,0x1,0x5,0x5,0x1,0x1,
    0x5,0x5,0x1,0x1,0x1,0x1,0x1,0x1,
    0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x6
  };

  BITMAP* backBuffer{ create_bitmap( TILE_BUFFER_WIDTH, TILE_BUFFER_HEIGHT ) };

  std::ifstream infile;
  infile.open( "st.prg", std::ios::binary );
  if( !infile.is_open() )
  {
    return -1;
  }

  int32_t xOffset { 0 };
  int32_t yOffset { 0 };
  const int32_t bit{ 0x80 };

  infile.seekg( 2, std::ios::beg );

  // --------------------------------------------
  // 1 byte = 8 pixels
  for( int32_t k = 0; k < NUM_TILES; k++ )
  {
    const int32_t tileColor{ tileColors[k] };

    int32_t posX = 0;
    int32_t posY = 0;

    for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
    {
      const int32_t val{ infile.get() };
      for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
      {
        const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0] ) };
        putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
      }

      posX = 0;
      ++posY;
    }

    posX = PIXELS_PER_BYTE;
    posY = 0;

    for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
    {
      const int32_t val{ infile.get() };
      for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
      {
        const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0] ) };
        putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
      }

      posX = 8;
      ++posY;
    }

    posX = 0;
    posY = PIXELS_PER_BYTE;

    for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
    {
      const int32_t val{ infile.get() };
      for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
      {
        const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0] ) };
        putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
      }

      posX = 0;
      ++posY;
    }

    posX = PIXELS_PER_BYTE;
    posY = PIXELS_PER_BYTE;

    for( int32_t j = 0; j < PIXELS_PER_QUAD; ++j )
    {
      const int32_t val{ infile.get() };
      for( int32_t i = 0; i < PIXELS_PER_BYTE; ++i )
      {
        const int32_t color{ ( val & ( bit >> i ) ? c64Colors[tileColor] : c64Colors[0] ) };
        putpixel( backBuffer, xOffset + posX++, yOffset + posY, color );
      }

      posX = PIXELS_PER_BYTE;
      ++posY;
    }

    xOffset += TILE_WIDTH;
    if( xOffset >= TILE_BUFFER_WIDTH )
    {
      xOffset = 0;
      yOffset += TILE_HEIGHT;
    }

    if( infile.eof() )
    {
      break;
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

  return 0;
}
