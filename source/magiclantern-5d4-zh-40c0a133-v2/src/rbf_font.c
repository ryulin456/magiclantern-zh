
#include "dryos.h"
#include "bmp.h"
#include "beep.h"
#include "rbf_font.h"

extern uint32_t ml_refresh_display_needed;

/* here some macros to wrap the CHDK functions for ML code */
#define FG_COLOR(f) FONT_FG(f)
#define BG_COLOR(f) FONT_BG(f)
#define MAKE_COLOR(fg,bg) FONT(0,fg,bg)

#define FONT_CONDENSED 0x00100000 /* this bit is not (yet) used, so we use it as internal flag */

#define draw_char(x,y,c,h) do{}while(0)

static int rbf_font_load(char *file, font* f, int maxchar);
static inline int rbf_font_height(font *rbf_font);
static inline int rbf_char_width(font *rbf_font, int ch);

//-------------------------------------------------------------------
static int RBF_HDR_MAGIC1 = 0x0DF00EE0;
static int RBF_HDR_MAGIC2 = 0x00000003;

struct font font_dynamic[MAX_DYN_FONTS+1];
static char *dyn_font_name[MAX_DYN_FONTS+1];
uint32_t dyn_fonts = 0;

bfnt_font *BFNT_ALT_FONT = 0;
uint8_t *BFNT_ALT_CHAR_CODES = 0;
uint8_t *BFNT_ALT_BITMAP_OFFSET = 0;
uint8_t *BFNT_ALT_BITMAP_DATA = 0;

#ifdef CONFIG_NO_BFNT
/* kitor: Those replace entries in platform consts.h for DIGIC6+
 *        that does not use BMP fonts anymore.  */
uint8_t *BFNT_CHAR_CODES;
uint8_t *BFNT_BITMAP_OFFSET;
uint8_t *BFNT_BITMAP_DATA;

bfnt_font* BFNT_FONT;
#endif

//-------------------------------------------------------------------

static font *new_font() {
    // allocate font from cached memory
    font *f = malloc(sizeof(font));
    if (f) {
        memset(f,0,sizeof(font));      // wipe memory
        // return address in cached memory
        return f;
    }

    // memory not allocated ! should probably do something else in this case ?
    return 0;
}

uint32_t font_by_name(char *file, uint32_t fg_color, uint32_t bg_color)
{
    /* check if this font was already loaded */
    for(int pos = 0; pos < (int)dyn_fonts; pos++)
    {
        if(!strcmp(file, dyn_font_name[pos]))
        {   
            /* yeah, so return a new specifier for this id */
            return FONT_DYN(pos, fg_color, bg_color);
        }
    }
    
    /* too many fonts loaded, return default font */
    if(dyn_fonts >= MAX_DYN_FONTS)
    {
        beep();
        bmp_printf(FONT_CANON, 0, 0, "Too many fonts");
        return FONT_CANON;
    }
    
    /* was not loaded, try to load */
    char filename[128];
    snprintf(filename, sizeof(filename), "ML/FONTS/%s.RBF", file);
    
    uint32_t size;
    if((FIO_GetFileSize( filename, &size ) != 0) || (size == 0))
    {
        /* failed to load, return default font */
        beep();
        bmp_printf(FONT_CANON, 0, 0, "%s not found", file);
        return FONT_CANON;
    }
    
    void *font = new_font();
    
    /* load the font here */
    if(!rbf_font_load(filename, font, 0))
    {
        bmp_printf(FONT_CANON, 0, 0, "%s not loaded", file);
        free(font);
        return FONT_CANON;
    }

    /* now updated cached font name (not filename) */
    dyn_font_name[dyn_fonts] = malloc(strlen(filename) + 1);
    if (!dyn_font_name[dyn_fonts])
    {
        free(((struct _font *) font)->cTable);
        free(font);
        return FONT_CANON;
    }
    strcpy(dyn_font_name[dyn_fonts], file);
    
    /* and measure font sizes */
    font_dynamic[dyn_fonts].bitmap = font;
    font_dynamic[dyn_fonts].height = rbf_font_height((void*)font_dynamic[dyn_fonts].bitmap);
    font_dynamic[dyn_fonts].width = rbf_char_width((void*)font_dynamic[dyn_fonts].bitmap, '0');
    dyn_fonts++;

    return FONT_DYN(dyn_fonts - 1, fg_color, bg_color);
}

// Return address of a validated RBF character bitmap.
static inline char* FAST rbf_font_char(font* f, int ch)
{
    if (f && f->cTable && ch >= f->hdr.charFirst && ch <= f->hdr.charLast)
        return &f->cTable[(ch - f->hdr.charFirst) * f->hdr.charSize];
    return 0;
}

static int rbf_valid_header(const font_hdr *h, unsigned int size)
{
    if (size < sizeof(*h) || h->magic1 != RBF_HDR_MAGIC1 ||
        h->magic2 != RBF_HDR_MAGIC2 || h->height < 1 || h->height > 64 ||
        h->charSize < 1 || h->charSize > 1024 || h->charSize % h->height ||
        h->charFirst < 0 || h->charLast > 255 || h->charLast < h->charFirst ||
        h->_wmapAddr < (int) sizeof(*h) || h->_cmapAddr < (int) sizeof(*h))
        return 0;
    unsigned int count = h->charLast - h->charFirst + 1;
    return (unsigned int) h->_wmapAddr <= size &&
           count <= size - h->_wmapAddr &&
           (unsigned int) h->_cmapAddr <= size &&
           count * h->charSize <= size - h->_cmapAddr;
}

static int rbf_font_load(char *file, font* f, int maxchar)
{
    if (!f)
        return 0;
    int size = 0;
    uint8_t *data = read_entire_file_persistent(file, &size);
    if (!data)
        return 0;
    if (size < (int) sizeof(font_hdr))
        goto fail;

    font_hdr h;
    memcpy(&h, data, sizeof(h));
    if (!rbf_valid_header(&h, size) ||
        (maxchar && (maxchar < h.charFirst || maxchar > h.charLast)))
        goto fail;
    if (maxchar)
        h.charLast = maxchar;

    int count = h.charLast - h.charFirst + 1;
    int bytes = count * h.charSize;
    char *table = malloc(bytes);
    if (!table)
        goto fail;
    memcpy(table, data + h._cmapAddr, bytes);
    if (f->cTable)
        free(f->cTable);
    f->hdr = h;
    f->width = 8 * h.charSize / h.height;
    f->charCount = count;
    f->cTable = table;
    f->cTableSize = f->cTableSizeMax = bytes;
    memset(f->wTable, 0, sizeof(f->wTable));
    memcpy(&f->wTable[h.charFirst], data + h._wmapAddr, count);
    f->wTable['\t'] = f->wTable[' '] * 4;
    ml_free_persistent(data);
    return 1;
fail:
    ml_free_persistent(data);
    return 0;
}

//-------------------------------------------------------------------
static inline int rbf_font_height(font *rbf_font) {
    return rbf_font->hdr.height;
}
//-------------------------------------------------------------------
static inline int rbf_char_width(font *rbf_font, int ch) {
    if (!ch)
        return 0;

    if (rbf_font->cTable && ch >= 0 && ch < 256)
        return (uint8_t) rbf_font->wTable[ch];

    return bfnt_char_get_width_scaled(ch, rbf_font->hdr.height);
}

//-------------------------------------------------------------------
/* Decode one UTF-8 sequence into Canon BFNT's packed-byte key.  Language
 * resources are fully UTF-8 validated before this renderer is reached. */
static int rbf_utf8_decode(const char **text)
{
    const uint8_t *s = (const uint8_t *) *text;
    uint8_t lead = s[0];
    if (lead < 0x80)
    {
        *text = (const char *) (s + 1);
        return lead;
    }

    int length = (lead >= 0xc2 && lead <= 0xdf) ? 2 :
                 (lead >= 0xe0 && lead <= 0xef) ? 3 :
                 (lead >= 0xf0 && lead <= 0xf4) ? 4 : 0;
    if (!length)
    {
        *text = (const char *) (s + 1);
        return 0;
    }

    /* Never read past the terminating NUL.  On malformed input, consume only
     * the bad lead byte so the following byte can be decoded independently. */
    for (int i = 1; i < length; i++)
    {
        if (!s[i] || (s[i] & 0xc0) != 0x80)
        {
            *text = (const char *) (s + 1);
            return 0;
        }
    }

    /* Reject overlong encodings, UTF-16 surrogate values and values beyond
     * Unicode's upper bound. */
    if ((lead == 0xe0 && s[1] < 0xa0) ||
        (lead == 0xed && s[1] >= 0xa0) ||
        (lead == 0xf0 && s[1] < 0x90) ||
        (lead == 0xf4 && s[1] >= 0x90))
    {
        *text = (const char *) (s + 1);
        return 0;
    }

    uint32_t packed = lead;
    for (int i = 1; i < length; i++)
        packed |= (uint32_t) s[i] << (8 * i);
    *text = (const char *) (s + length);

    if (length == 4) /* no four-byte glyphs in this BFNT */
        return 0;
    return packed;
}

int rbf_str_width(font *rbf_font, const char *str) {
    int l=0;
    int maxl = 0;

    // Calculate how long the string is in pixels
    // and return the length of the longest line
    while (*str)
    {
        const char *next = str;
        int ch = rbf_utf8_decode(&next);
        str = next;

        if (ch == '\n')
        {
            maxl = MAX(l, maxl);
            l = 0;
        }
        else if (ch)
        {
            l += rbf_char_width(rbf_font, ch);
        }
    }
    maxl = MAX(l, maxl);

    return maxl;
}

int rbf_str_clipped_width(font *rbf_font, const char *str, int maxlen) {
    int l = 0;
    // Calculate how long the string is in pixels (possibly clipped to 'maxlen')
    while (*str)
    {
        const char *next = str;
        int ch = rbf_utf8_decode(&next);
        if (!ch)
        {
            str = next;
            continue;
        }
        int width = rbf_char_width(rbf_font, ch);
        if (l + width > maxlen)
            break;
        l += width;
        str = next;
    }

    return l;
}

int rbf_strlen_clipped(font *rbf_font, const char *str, int maxlen) {
    int l = 0;
    // Calculate how long the string is in characters (possibly clipped to 'maxlen')
    char* str0 = (char*) str;
    while (*str)
    {
        const char *next = str;
        int ch = rbf_utf8_decode(&next);
        if (!ch)
        {
            str = next;
            continue;
        }
        int width = rbf_char_width(rbf_font, ch);
        if (l + width > maxlen)
            break;
        l += width;
        str = next;
    }

    return str - str0;
}

//-------------------------------------------------------------------
static void FAST font_draw_char(font *rbf_font, int x, int y, char *cdata, int width, int height, int pixel_width, int fontspec) {
    int xx, yy;
    uint8_t * bmp = bmp_vram();
    int fg = FG_COLOR(fontspec);
    int bg = BG_COLOR(fontspec);
    int x0 = fontspec & FONT_CONDENSED ? 1 : 0;
    
    // draw pixels for font character
    if (cdata)
    {
        if (bg != NO_BG_ERASE)
        {
            bmp_fill(bg, x, y, pixel_width, height);
        }

        for (yy=0; yy<height; ++yy)
        {
            if (y+yy <= BMP_H_MINUS || y+yy >= BMP_H_PLUS)
            {
                break;
            }
            for (xx=x0; xx<pixel_width; ++xx)
            {
                if(cdata[yy*width/8+xx/8] & (1<<(xx%8)))
                {
                    bmp_putpixel_fast(bmp, x+xx, y+yy, fg);
                }
            }
        }
    }
}

static void FAST font_draw_char_shadow(font *rbf_font, int x, int y, char *cdata, int width, int height, int pixel_width, int fontspec) {
    int xx, yy;
    uint8_t * bmp = bmp_vram();
    int fg = FG_COLOR(fontspec);
    int bg = BG_COLOR(fontspec);
    
    // draw pixels for font character
    if (cdata)
    {
        for (yy=0; yy<height; ++yy)
        {
            if (y+yy <= BMP_H_MINUS || y+yy >= BMP_H_PLUS)
            {
                break;
            }
            for (xx=0; xx<pixel_width; ++xx)
            {
                int px = (cdata[yy*width/8+xx/8] & (1<<(xx%8)));
                if (px)
                {
                    bmp_putpixel_fast(bmp, x+xx, y+yy, fg);
                    
                    /* shadow: background pixels are only drawn near a foreground pixel */
                    /* heuristic: usually there are much less fg pixels than bg, so it makes sense to look for neighbours here */
                    for (int xxx = MAX(xx-1, 0); xxx <= MIN(xx+1, pixel_width-1); xxx++)
                    {
                        for (int yyy = MAX(yy-1, 0); yyy <= MIN(yy+1, height-1); yyy++)
                        {
                            int pxx = (cdata[yyy*width/8+xxx/8] & (1<<(xxx%8)));
                            if (!pxx)
                            {
                                bmp_putpixel_fast(bmp, x+xxx, y+yyy, bg);
                            }
                        }
                    }
                }
            }
        }
    }
}

//-------------------------------------------------------------------
static int FAST rbf_draw_char(font *rbf_font, int x, int y, int ch, int fontspec) {
    // Get char data pointer
    char* cdata = (rbf_font->cTable && ch >= 0 && ch < 256)
        ? rbf_font_char(rbf_font, ch)
        : 0;
    int width;

    if (!rbf_font->cTable || ch < 0 || ch >= 256)
        width = bfnt_draw_char_scaled(ch, x, y, FG_COLOR(fontspec), BG_COLOR(fontspec), rbf_font->hdr.height);
    else if (fontspec & SHADOW_MASK)
    {
        font_draw_char_shadow(rbf_font, x, y, cdata, rbf_font->width, rbf_font->hdr.height, rbf_font->wTable[ch], fontspec);
        width = (uint8_t) rbf_font->wTable[ch];
    }
    else
    {
        font_draw_char(rbf_font, x, y, cdata, rbf_font->width, rbf_font->hdr.height, rbf_font->wTable[ch], fontspec);
        width = (uint8_t) rbf_font->wTable[ch];
    }

#if 0   /* fixme: breaks cursor in editor.lua */
    if (ch == '\t')
    {
        int tab_width = rbf_font->wTable[' '] * 4;
        return (x + tab_width) / tab_width * tab_width - x;
    }
#endif
    ml_refresh_display_needed = 1;
    return width;
}


//-------------------------------------------------------------------
// Draw a string colored 'c1' with the character at string-position 'c' colored 'c2'.
static int rbf_draw_string_c(font *rbf_font, int x, int y, const char *str, int fontspec1, int c, int fontspec2) {
     int l=0, i=0;

     while (*str) {
          int ch = rbf_utf8_decode(&str);
          if (ch == '\n')
          {
              l = 0;
              y += rbf_font->hdr.height;
              i++;
              continue;
          }
          if (!ch)
              continue;
          l+=rbf_draw_char(rbf_font, x+l, y, ch, (i==c)?fontspec1:fontspec2);
          ++i;
     }
     return l;
}

//-------------------------------------------------------------------
static int rbf_draw_string_simple(font *rbf_font, int x, int y, const char *str, int fontspec) {
    return rbf_draw_string_c(rbf_font, x, y, str, fontspec, -1, fontspec);
}

//-------------------------------------------------------------------
static int rbf_draw_clipped_string(font *rbf_font, int x, int y, const char *str, int fontspec, int maxlen)
{
    int l = 0;
    
    int justified = (fontspec & FONT_ALIGN_MASK) == FONT_ALIGN_JUSTIFIED;
    
    if (justified)
    {
        int should_fill = !(fontspec & SHADOW_MASK);
        int bg = FONT_BG(fontspec);
        int len = maxlen;
        int space = len - rbf_str_width(rbf_font, str);
        int is_mono = rbf_char_width(rbf_font, 'm') == rbf_char_width(rbf_font, 'i');
        
        /* divide the space across the chars: a space character can accept 5 times more stretch space than a regular letter */
        /* first non-letter (indent) can't be stretched (to render bullet points correctly) */
        /* monospaced fonts are condensed uniformly */
        int bins = 0;
        int glyphs = 0;
        const char *scan = str;
        int indent = 1;
        while (*scan)
        {
            int ch = rbf_utf8_decode(&scan);
            if (ch)
                glyphs++;
        }

        scan = str;
        int seen = 0;
        while (*scan)
        {
            int ch = rbf_utf8_decode(&scan);
            if (!ch)
                continue;
            seen++;
            if (seen == glyphs)
                break; /* last glyph should not be stretched */
            if (ch != ' ' && ch != '*') indent = 0;
            bins += is_mono ? 3 : indent ? 0 : space < 0 ? 1 : ch == ' ' ? 10 : 2;
        }
        
        /* use Bresenham line-drawing algorithm to divide space with integer-only math */
        /* http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#Algorithm_with_Integer_Arithmetic */
        /* x: from 0 to bins */
        /* y: space accumulated (from 0 to space) */
        int dx = MAX(bins, 1);
        int dy = ABS(space);
        
        /* if it requires too much stretching, just stretch as much as we can */
        dy = MIN(dy, dx);
        
        int D = 2*dy - dx;

        // Draw chars from string up to max pixel length
        indent = 1;
        int condensed = 0;
        seen = 0;
        while (*str)
        {
            const char *next = str;
            int ch = rbf_utf8_decode(&next);
            if (!ch)
            {
                str = next;
                continue;
            }
            int width = rbf_char_width(rbf_font, ch);
            if (l + width > maxlen)
                break;

            l += rbf_draw_char(rbf_font, x+l, y, ch, fontspec | (condensed ? FONT_CONDENSED : 0) );
            int l0 = l;
            seen++;

            if (seen < glyphs && bins)
            {
                /* Bresenham step */
                if (ch != ' ' && ch != '*') indent = 0;
                int repeat = is_mono ? 3 : indent ? 0 : space < 0 ? 1 : ch == ' ' ? 10 : 2;
                condensed = 0;
                for (int i = 0; i < repeat; i++)
                {
                    if (D > 0)
                    {
                        l += SGN(space);
                        D = D + (2*dy - 2*dx);
                        if (space < 0) condensed = 1;
                    }
                    else
                    {
                        D = D + 2*dy;
                    }
                }
            }
            
            if (should_fill && l > l0)
            {
                bmp_fill(bg, x+l0, y, l-l0, rbf_font->hdr.height);
            }
            
            str = next;
        }
    }
    else
    {
        // Draw chars from string up to max pixel length
        while (*str)
        {
            const char *next = str;
            int ch = rbf_utf8_decode(&next);
            if (!ch)
            {
                str = next;
                continue;
            }
            int width = rbf_char_width(rbf_font, ch);
            if (l + width > maxlen)
                break;
            l += rbf_draw_char(rbf_font, x+l, y, ch, fontspec);
            str = next;
        }
    }
    return l;
}

//-------------------------------------------------------------------
static int rbf_draw_string_single_line(font *rbf_font, int x, int y, const char *str, int fontspec, int maxlen) {
    
    /* how much space do we have to draw the string? */
    int len = maxlen;
    
    // Calulate amount of padding needed
    int padding = len - rbf_str_clipped_width(rbf_font, str, len);

    /* is that padding on the left? on the right? or both? */
    int padding_left = 0;
    int padding_right = 0;
    
    switch (fontspec & FONT_ALIGN_MASK)
    {
        case FONT_ALIGN_LEFT:
            padding_right = padding;
            break;
        
        case FONT_ALIGN_CENTER:
            x -= len/2;
            padding_left = padding/2;
            padding_right = padding - padding/2;
            break;
        
        case FONT_ALIGN_RIGHT:
            padding_left = padding;
            x -= len;
            break;
        
        case FONT_ALIGN_JUSTIFIED:
            padding_left = 0;
            padding_right = 0;
    }
    
    int bg = FONT_BG(fontspec);
    int should_fill = fontspec & FONT_ALIGN_FILL;

    // Fill left padding with background color
    if (should_fill && padding_left)
        bmp_fill(bg, x, y, padding_left, rbf_font->hdr.height);

    // Draw chars
    rbf_draw_clipped_string(rbf_font, x + padding_left, y, str, fontspec, len - padding_right - padding_left);

    // Fill right padding with background color
    if (should_fill && padding_right)
        bmp_fill(bg, x+len-padding_right, y, padding_right, rbf_font->hdr.height);

    return len;
}

int rbf_draw_string(font *rbf_font, int x, int y, const char *str, int fontspec) {

    /* how much space do we have to draw the string? */
    int len = FONT_GET_TEXT_WIDTH(fontspec);

    /* choose some reasonable defaults for text width, if it's not specified */
    if (len == 0)
    {
        if ((fontspec & FONT_ALIGN_MASK) == FONT_ALIGN_LEFT && (fontspec & FONT_ALIGN_FILL) == 0)
        {
            /* no fancy alignment */
            return rbf_draw_string_simple(rbf_font, x, y, str, fontspec);
        }
        else
        {
            /* use natural string length */
            len = rbf_str_width(rbf_font, str);
        }
    }

    char* start = (char*) str;
    char* end = start;

    /* for each line in string */
    while (*start)
    {
        /* where does this line end? */
        while (*end && *end != '\n')
            end++;
        
        /* chop the string here */
        char old = *end;
        *end = 0;
        
        /* draw this line */
        rbf_draw_string_single_line(rbf_font, x, y, start, fontspec, len);
        
        /* finished? */
        if (old == 0)
            break;
        
        /* undo chopping */
        *end = old;
        
        /* okay, let's go to next line */
        y += rbf_font->hdr.height;
        start = end = end+1;
    }
    
    return len;
}


/* for compatibility with existing code */
struct font font_small;
struct font font_med;
struct font font_med_large;
struct font font_large;
struct font font_canon;


static int bfnt_validate_card_font(bfnt_font *font, uint32_t size)
{
    int name_terminated = 0;
    if (font && size >= sizeof(*font))
    {
        for (uint32_t i = 0; i < sizeof(font->name); i++)
            if (!font->name[i])
            {
                name_terminated = 1;
                break;
            }
    }

    if (!font || size < sizeof(*font) || font->magic != 0x00544e46 ||
        !font->font_width || font->font_width > 64 ||
        font->charmap_offset < sizeof(bfnt_font) ||
        (font->charmap_offset & 3) ||
        !name_terminated ||
        (font->charmap_size & 3) ||
        font->charmap_offset > size)
        return 0;

    uint32_t remaining = size - font->charmap_offset;
    if (font->charmap_size > remaining / 2)
        return 0;
    uint32_t bitmap_start = font->charmap_offset + 2 * font->charmap_size;
    if (font->bitmap_size > size - bitmap_start)
        return 0;

    uint32_t count = font->charmap_size / 4;
    if (count < 95)
        return 0;
    uint32_t *codes = (uint32_t *) ((uint8_t *) font + font->charmap_offset);
    uint32_t *offsets = (uint32_t *) ((uint8_t *) codes + font->charmap_size);
    uint8_t *bitmap = (uint8_t *) font + bitmap_start;
    uint32_t ascii = 0x20;

    for (uint32_t i = 0; i < count; i++)
    {
        if ((i < 95 && codes[i] != i + 0x20) ||
            (!i && offsets[i] != 0) ||
            (i && codes[i - 1] >= codes[i]) ||
            (i && offsets[i - 1] >= offsets[i]) ||
            (offsets[i] & 1) || offsets[i] > font->bitmap_size ||
            font->bitmap_size - offsets[i] < 10)
            return 0;

        uint16_t *glyph = (uint16_t *) (bitmap + offsets[i]);
        uint32_t width = glyph[0];
        uint32_t height = glyph[1];
        uint32_t row_bytes = (width + 7) / 8;
        if (width > 100 || height > 64 ||
            row_bytes * height > font->bitmap_size - offsets[i] - 10)
            return 0;

        if (codes[i] == ascii && ascii <= 0x7e)
            ascii++;
    }

    return ascii == 0x7f;
}

static int bfnt_load_alt_from_card()
{
    const char *filename = "ML/FONTS/zh_cn.bfn";
    uint32_t size;

    if (FIO_GetFileSize(filename, &size) != 0 || size < sizeof(bfnt_font))
        return 1;

    /* This font remains resident.  Keep it out of the temporary/DMA pools
     * used by Canon shooting and menu tasks. */
    BFNT_ALT_FONT = (bfnt_font *) read_entire_file_persistent(filename, (int *) &size);
    if (!BFNT_ALT_FONT)
        return 2;

    if (!bfnt_validate_card_font(BFNT_ALT_FONT, size))
    {
        ml_free_persistent(BFNT_ALT_FONT);
        BFNT_ALT_FONT = 0;
        return 3;
    }

    BFNT_ALT_CHAR_CODES = (uint8_t *) BFNT_ALT_FONT + BFNT_ALT_FONT->charmap_offset;
    BFNT_ALT_BITMAP_OFFSET = BFNT_ALT_CHAR_CODES + BFNT_ALT_FONT->charmap_size;
    BFNT_ALT_BITMAP_DATA = BFNT_ALT_BITMAP_OFFSET + BFNT_ALT_FONT->charmap_size;
    return 0;
}


#ifdef CONFIG_NO_BFNT
//-------------------------------------------------------------------
// Load bitmap.bfn from card
static int bfnt_load_from_card()
{
    char filename[128];
    //TODO? make it configurable?
    snprintf(filename, sizeof(filename), "ML/FONTS/bitmap.bfn");
    uint32_t size;
    if (FIO_GetFileSize(filename, &size) != 0 || size < sizeof(bfnt_font))
        return 1;

    DryosDebugMsg(0, 15, "File '%s' size %d bytes", filename, size);

    BFNT_FONT = (bfnt_font*)read_entire_file_persistent(filename, (int*)&size);
    if( BFNT_FONT == NULL ){
        DryosDebugMsg(0, 15, "BFNT read failed.");
        return 2;
    }
    DryosDebugMsg(0, 15, "read ok");

    if (!bfnt_validate_card_font(BFNT_FONT, size))
    {
        DryosDebugMsg(0, 15, "Font magic incorrect: 0x%08x", BFNT_FONT->magic);
        ml_free_persistent(BFNT_FONT);
        BFNT_FONT = 0;
        return 3;
    }

    return 0;
}
#endif

/* An embedded, build-time-reviewed ASCII font remains available even if all
 * card fonts are missing or invalid. No allocator or card access is needed. */
__asm__(".pushsection .rodata\n"
        ".balign 4\n"
        ".global ml_fallback_rbf_start\n"
        "ml_fallback_rbf_start:\n"
        ".incbin \"../../data/fonts/term20.rbf\"\n"
        ".popsection\n");
extern const uint8_t ml_fallback_rbf_start[];
static font emergency_font;

static void init_emergency_font(void)
{
    const uint8_t *data = ml_fallback_rbf_start;
    memcpy(&emergency_font.hdr, data, sizeof(font_hdr));
    font_hdr *h = &emergency_font.hdr;
    emergency_font.width = 8 * h->charSize / h->height;
    emergency_font.charCount = h->charLast - h->charFirst + 1;
    emergency_font.cTable = (char *) data + h->_cmapAddr;
    memcpy(&emergency_font.wTable[h->charFirst], data + h->_wmapAddr,
           emergency_font.charCount);
    emergency_font.wTable['\t'] = emergency_font.wTable[' '] * 4;
    for (int i = 0; i <= MAX_DYN_FONTS; i++)
    {
        font_dynamic[i].bitmap = (void *) &emergency_font;
        font_dynamic[i].height = h->height;
        font_dynamic[i].width = emergency_font.wTable['0'];
    }
}

/* must be called before menu_init, otherwise it can't measure strings */
void _load_fonts()
{
    /* tolerate multiple calls, but only run the first */
    static int fonts_loaded = 0;
    if (fonts_loaded)
        return;
    fonts_loaded = 1;
    init_emergency_font();

    int alt_bfnt_status = bfnt_load_alt_from_card();
    DryosDebugMsg(0, 15, "alternate BFNT status: %d", alt_bfnt_status);

    int bfnt_status = -1;

    #ifdef CONFIG_NO_BFNT
    #ifdef CONFIG_5D4
    /* Do not let a bitmap.bfn left by another camera replace the reviewed
     * package font on 5D4.  The validated zh_cn.bfn contains ASCII and CJK. */
    if (alt_bfnt_status == 0)
    {
        BFNT_FONT = BFNT_ALT_FONT;
        BFNT_CHAR_CODES = BFNT_ALT_CHAR_CODES;
        BFNT_BITMAP_OFFSET = BFNT_ALT_BITMAP_OFFSET;
        BFNT_BITMAP_DATA = BFNT_ALT_BITMAP_DATA;
        bfnt_status = 0;
        DryosDebugMsg(0, 15, "using package BFNT as primary font");
    }
    #else
    //Try to load BFNT from card on newer generations
    bfnt_status = bfnt_load_from_card();
    if (bfnt_status != 0 && alt_bfnt_status == 0)
    {
        /* 5D4/D6 has no Canon BFNT in ROM and this package intentionally
         * carries one validated UTF-8 font instead of a second bitmap.bfn.
         * Reuse the same persistent allocation for ASCII and CJK so
         * FONT_CANON remains measurable and drawable without shoot_malloc. */
        BFNT_FONT = BFNT_ALT_FONT;
        BFNT_CHAR_CODES = BFNT_ALT_CHAR_CODES;
        BFNT_BITMAP_OFFSET = BFNT_ALT_BITMAP_OFFSET;
        BFNT_BITMAP_DATA = BFNT_ALT_BITMAP_DATA;
        bfnt_status = 0;
        DryosDebugMsg(0, 15, "using alternate BFNT as primary font");
    }
    #endif
    if ( bfnt_status == 0) {
        DryosDebugMsg(0, 15, "bfnt read OK: %08x %s",  BFNT_FONT, BFNT_FONT->name);
        DryosDebugMsg(0, 15, "hdr %08x char %08x", BFNT_FONT->charmap_offset, BFNT_FONT->charmap_size);

        //setup variables that would be constants from ROM on old generations
        BFNT_CHAR_CODES    = (uint8_t *)BFNT_FONT + BFNT_FONT->charmap_offset;
        BFNT_BITMAP_OFFSET = BFNT_CHAR_CODES + BFNT_FONT->charmap_size;
        BFNT_BITMAP_DATA   = BFNT_BITMAP_OFFSET + BFNT_FONT->charmap_size;

        DryosDebugMsg(0, 15, "%08x %08x %08x", BFNT_CHAR_CODES, BFNT_BITMAP_OFFSET, BFNT_BITMAP_DATA);
    }
    else{
        DryosDebugMsg(0, 15, "bfnt read fail: %d", bfnt_status);
    }
    #else
    //If constants are not zero - assume BFNT is available from ROM
    if(BFNT_CHAR_CODES && BFNT_BITMAP_OFFSET && BFNT_BITMAP_DATA)
        bfnt_status = 0;
    #endif

    /* fake font for Canon BFNT font backend, with the same metrics */
    static font canon_font_storage;
    font *canon_font = &canon_font_storage;
    canon_font->hdr.height = 40;
    for (int i = 0; i < 256; i++)
        canon_font->wTable[i] = bfnt_char_get_width(i);

    /* Try to load some RBF fonts */
    font_by_name("term12", COLOR_BLACK, COLOR_WHITE);
    font_by_name("term20", COLOR_BLACK, COLOR_WHITE);
    #ifdef CONFIG_LOW_RESOLUTION_DISPLAY
    font_by_name("arghlf22", COLOR_BLACK, COLOR_WHITE);
    #else
    font_by_name("argnor23", COLOR_BLACK, COLOR_WHITE);
    #endif
    font_by_name("argnor28", COLOR_BLACK, COLOR_WHITE);
    font_by_name("argnor32", COLOR_BLACK, COLOR_WHITE);

    if (bfnt_status == 0) {
        /* use BFNT font as fallback */
        for (int i = dyn_fonts; i <= MAX_DYN_FONTS; i++)
        {
            font_dynamic[i].bitmap = (void*) canon_font;
            font_dynamic[i].height = 40;
            font_dynamic[i].width = rbf_char_width((void*)font_dynamic[i].bitmap, '0');
        }

        font_canon = *fontspec_font(FONT_CANON);
    }
    else {
        /* use last loaded RBF font as a fallback */
        font_by_name("argnor32", COLOR_BLACK, COLOR_WHITE);
        if(dyn_fonts)
        {
            for (int i = dyn_fonts; i <= MAX_DYN_FONTS; i++)
                font_dynamic[i] = font_dynamic[dyn_fonts - 1];
        }

        /* Replace missing FONT_CANON with FONT_LARGE */
        font_canon = *fontspec_font(FONT_LARGE);
    }

    font_small = *fontspec_font(FONT_SMALL);
    font_med = *fontspec_font(FONT_MED);
    font_med_large = *fontspec_font(FONT_MED_LARGE);
    font_large = *fontspec_font(FONT_LARGE);
}
