#include <pebble.h>

typedef struct Glyph
{
    GBitmap* small;
    GBitmap* big;
} Glyph;
typedef struct Word
{
    int start;
    int len;
} Word;
enum WordID
{
    WORD_PRE_FUNF=0,
    WORD_PRE_ZEHN,
    WORD_PRE_UND,
    WORD_PRE_ZWANZIG,
    WORD_PRE_VIERTEL,
    WORD_HALB,
    WORD_VOR,
    WORD_NACH,
    WORD_EIN,
    WORD_EINS,
    WORD_ZWEI,
    WORD_DREI,
    WORD_VIER,
    WORD_FUNF,
    WORD_SECHS,
    WORD_SIEBEN,
    WORD_ACHT,
    WORD_NEUN,
    WORD_ZEHN,
    WORD_ELF,
    WORD_ZWOLF,
    WORD_UHR,
    WORD_COUNT
};

#define COLS 9
#define ROWS 12
#define toON(c) c=((c>='a'&&c<='z')?c-('a'-'A'):c)
#define toOFF(c) c=((c>='A'&&c<='Z')?c+('a'-'A'):c)

Window* window;
GBitmap* bigImage;
GBitmap* smallImage;
Glyph glyphs[26];
char map[COLS*ROWS+1]="ESzISTafunftlUNDkzwanzigzehnpviertelnachvorohalbeinzwolfzweisiebenedreiefunfelfneunsvierachtzehnsechseinsuhr";
Word words[WORD_COUNT]= {{7,4},{24,4},{13,3},{17,7},{29,7},{44,4},{40,3},{36,4},{48,3},{101,4},{56,4},{67,4},{84,4},{72,4},{96,5},{60,6},{88,4},{79,4},{92,4},{76,3},{51,5},{105,3}};

void mark_word (int id,bool on) {
    int i;
    for(i=words[id].start;i<words[id].start+words[id].len;i++) {
        if (on)
            toON(map[i]);
        else
            toOFF(map[i]);
    }
}

void layer_update_callback(Layer *me, GContext* ctx) {
  GRect draw;
  int x,y,i;
  graphics_context_set_compositing_mode(ctx,GCompOpAssign);
  for (x=0;x<COLS;x++) {
      for (y=0;y<ROWS;y++) {
          draw.origin.x=x*14+9;
          draw.origin.y=y*14;
          i=(COLS-x-1)*ROWS+y; //looks wierd but thats the price to look like fez
          if (map[i]>='A'&&map[i]<='Z') {
              i=map[i]-'A'; //its not the glyph index
              draw.origin.x+=1;
              draw.origin.y+=1;
              draw.size.w=12;
              draw.size.h=12;
              graphics_draw_bitmap_in_rect(ctx,glyphs[i].big,draw);
          }
          else {
              i=map[i]-'a';
              draw.origin.x+=3;
              draw.origin.y+=3;
              draw.size.w=6;
              draw.size.h=6;
              graphics_draw_bitmap_in_rect(ctx,glyphs[i].small,draw);
          }
      }
  }
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    //TODO: set change bounds from 0->4 to -2->2
    int i;
    int hour=tick_time->tm_hour;
    int minute=tick_time->tm_min/5;
    for (i=0;i<WORD_COUNT;i++)
        mark_word(i,false);
    if (minute>5)
        hour++;
    if (hour>=12)
        hour-=12;
    hour=(hour==0?WORD_ZWOLF:hour+WORD_EIN);
    if (hour==WORD_EINS&&minute==0)
        hour--; //it's not "ES IST EINS UHR", it is "ES IST EIN UHR"
    mark_word(hour,true);
    switch (minute) {
        case(0):{mark_word(WORD_UHR,true);}break;//0-4
        case(1):{mark_word(WORD_PRE_FUNF,true);mark_word(WORD_NACH,true);}break;//5-9
        case(2):{mark_word(WORD_PRE_ZEHN,true);mark_word(WORD_NACH,true);}break;//10-14
        case(3):{mark_word(WORD_PRE_VIERTEL,true);mark_word(WORD_NACH,true);}break;//15-19
        case(4):{mark_word(WORD_PRE_ZWANZIG,true);mark_word(WORD_NACH,true);}break;//20-24
        case(5):{mark_word(WORD_PRE_FUNF,true);mark_word(WORD_PRE_UND,true);mark_word(WORD_PRE_ZWANZIG,true);mark_word(WORD_NACH,true);}break;//25-29
        case(6):{mark_word(WORD_HALB,true);}break;//30-34
        case(7):{mark_word(WORD_PRE_FUNF,true);mark_word(WORD_PRE_UND,true);mark_word(WORD_PRE_ZWANZIG,true);mark_word(WORD_VOR,true);}break;//35-39
        case(8):{mark_word(WORD_PRE_ZWANZIG,true);mark_word(WORD_VOR,true);}break;//40-44
        case(9):{mark_word(WORD_PRE_VIERTEL,true);mark_word(WORD_VOR,true);}break;//45-49
        case(10):{mark_word(WORD_PRE_ZEHN,true);mark_word(WORD_VOR,true);}break;//50-54
        case(11):{mark_word(WORD_PRE_FUNF,true);mark_word(WORD_VOR,true);}break;//55-59
        default:{return;}break;
    }
    layer_mark_dirty (window_get_root_layer(window));
}

void handle_init () {
    //Load resources
    GRect subSmall={{0,0},{6,6}};
    GRect subBig={{0,0},{12,12}};
    int i;
    bigImage=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FEZ);
    smallImage=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FEZ_SMALL);
    for (i=0;i<26;i++) {
        glyphs[i].big=gbitmap_create_as_sub_bitmap (bigImage,subBig);
        glyphs[i].small=gbitmap_create_as_sub_bitmap (smallImage,subSmall);
        subBig.origin.x+=subBig.size.w;
        subSmall.origin.x+=subSmall.size.w;
        if (subBig.origin.x==8*subBig.size.w) {
            subBig.origin.x=0;
            subBig.origin.y+=subBig.size.h;
            subSmall.origin.x=0;
            subSmall.origin.y+=subSmall.size.h;
        }
    }

  //Initialise window
  window=window_create();
  window_set_background_color(window, GColorWhite);
  window_set_fullscreen(window,true);
  window_set_status_bar_icon(window,glyphs['f'-'a'].big);
  layer_set_update_proc (window_get_root_layer(window),layer_update_callback);

  window_stack_push(window, true);

  time_t timeSec=time(0);
  struct tm* time=localtime(&timeSec);
  handle_minute_tick(time,0);
  tick_timer_service_subscribe(MINUTE_UNIT,handle_minute_tick);
}


void handle_deinit() {
  for (int i=0;i<26;i++) {
    gbitmap_destroy(glyphs[i].big);
    gbitmap_destroy(glyphs[i].small);
  }
  gbitmap_destroy(bigImage);
  gbitmap_destroy(smallImage);
  window_destroy(window);
}

int main(void) {
  handle_init ();
  app_event_loop();
  handle_deinit ();
  return 0;
}
