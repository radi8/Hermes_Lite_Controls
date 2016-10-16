// Features are chosen here. Uncomment the desired features

//#define FEATURE_I2C_LCD
#define FEATURE_Quisk
//#define FEATURE_pihpsdr
//#define FEATURE_Custom_Filter
#define FEATURE_Invert_Inputs     // Allows inverting or non-inverting buffers from HL to Arduino
#define FEATURE_Invert_ptt_line   // Use for open collector driver where ptt tx is low active

// Debug Defines follow here
#define DEBUG_ENABLED            // Must enable this if any debug at all is enabled
#define DEBUG_BAND_CHANGE        // Check the values arriving on User0 to User3
#define DEBUG_PTT_CHANGE
#define DEBUG_SHOW_FILTER_SWITCH_SIGNALS

// Available Transmit Filters. Edit the bit associated with the filter to suit wiring of circuit.
// Up to 7 filters may be controlled with bit 0 being a special case where when it is released the
// roofing filter is switched in and when it is operated it is in thru mode where all the filters are
// bypassed. 
// bit 0
const uint8_t LP12_10 = 0;  // Bit 0 clear = 10M filter in circuit
const uint8_t LPthru  = 1;  // Bit 0 set = thru mode

const uint8_t LP160   = 2;  // Bit 1 set = 160M filter selected. Bit 1 clear = filter not selected
const uint8_t LP80    = 4;  // Bit 2 set = 160M filter selected. Bit 2 clear = filter not selected
const uint8_t LP60_40 = 8;  // Bit 3 set = 60/40M filter selected. Bit 3 clear = filter not selected
const uint8_t LP30_20 = 16; // Bit 4 set = 30/20M filter selected. Bit 4 clear = filter not selected
const uint8_t LP17_15 = 32; // Bit 5 set = 17/15M filter selected. Bit 5 clear = filter not selected


// Available Receive Filters. Edit the bit associated with the filter to suit wiring of circuit

const uint8_t HP160   = 0;  // Released = 160M. Bit 0 = Lo
const uint8_t HPthru  = 1;  // Operated = thru mode; Bit 0 = Hi
const uint8_t HP80_60 = 2;  // Operated = 80M/60.  Bit 1 = Hi
const uint8_t HP40_30 = 4;  // Operated = 40/30M  Bit 2 Hi
const uint8_t HP20_17 = 8;  // Operated = 20/17M. Bit 3 Hi
const uint8_t HP15_10 = 16; // Operated = 15-10M. Bit 4 Hi



// Here we build the map of what filters will be selected from the word formed
// by the bits on USER0,1,2,3 inputs. If 2 filters need to be set, they should
// be or'd together e.g. (LP12_10 || LP17_15)

#if defined(FEATURE_Quisk)

const uint8_t txFilterMap[16] = {
// Filter   Button  Band          Selected
// ------   ------  ----          --------  
  LP12_10,  // 0,   No band,      10 metre Lo Pass only filter selected
  LPthru,   // 1,   137 KHz band, thru filter selected
  LP160,    // 2,   160 M band,   LP160 plus LP12_10M filters selected
  LP80,     // 3,   80 M band,    LP80 plus LP12_10M filters selected
  LP60_40,  // 4,   60 M band,    LP60_40 plus LP12_10M filters selected
  LP60_40,  // 5,   40 M band,    LP60_40 plus LP12_10M filters selected
  LP30_20,  // 6,   30 M band,    LP30_20 plus LP12_10M filters selected
  LP30_20,  // 7,   20 M band,    LP30_20 plus LP12_10M filters selected
  LP17_15,  // 8,   17 M band,    LP17_15 plus LP12_10M filters selected
  LP17_15,  // 9,   15 M band,    LP17_15 plus LP12_10M filters selected
  LP12_10,  // 10,  12 M band,    10 metre Lo Pass only filter selected
  LP12_10,  // 11,  10 M band,    10 metre Lo Pass only filter selected
  LPthru,   // 12,   6 M band,    thru filter selected
  LPthru,   // 13,  Through pass. i.e. no filters with thru connection
  LPthru,   // 14,  Through pass. i.e. no filters with thru connection
  LPthru    // 15,  Through pass. i.e. no filters with thru connection
};

const uint8_t rxFilterMap[16] = {
// Filter   Button  Band          Selected
// ------   ------  ----          --------  
  HP160,    // 0,   No band,      HP160 only filter selected
  HPthru,   // 1,   137 KHz band, thru filter selected
  HP160,    // 2,   160 M band,   HP160 only filter selected
  HP80_60,  // 3,   80 M band,    HP160 and HP80_60 filters selected
  HP80_60,  // 4,   60 M band,    HP160 and HP80_60 filters selected
  HP40_30,  // 5,   40 M band,    HP160 and HP40_30 filters selected
  HP40_30,  // 6,   30 M band,    HP160 and HP40_30 filters selected
  HP20_17,  // 7,   20 M band,    HP160 and HP20_17 filters selected
  HP20_17,  // 8,   17 M band,    HP160 and HP20_17 filters selected
  HP15_10,  // 9,   15 M band,    HP160 and HP15_10 filters selected
  HP15_10,  // 10,  12 M band,    HP160 and HP15_10 filters selected
  HP15_10,  // 11,  10 M band,    HP160 and HP15_10 filters selected
  HP15_10,  // 12,   6 M band,    HP160 and HP15_10 filters selected
  HPthru,   // 13,  Through pass. i.e. no filters with thru connection
  HPthru,   // 14,  Through pass. i.e. no filters with thru connection
  HPthru    // 15,  Through pass. i.e. no filters with thru connection
};
#endif

#if defined(FEATURE_pihpsdr)

#endif

#if defined(FEATURE_Custom_Filter)

#endif
