// Features are chosen here. Uncomment the desired features

//#define FEATURE_I2C_LCD
#define FEATURE_Quisk
//#define FEATURE_pihpsdr
//#define FEATURE_Custom_Filter
#define FEATURE_Invert_Inputs    // Allows inverting or non-inverting buffers from HL to Arduino

// Debug Defines follow here
//#define DEBUG_LEVEL_1
//#define DEBUG_LEVEL_2
#define DEBUG_LEVEL_3

// Available Transmit Filters. Edit the bit associated with the filter to suit wiring of circuit

const uint8_t LP12_10 = 0;  // Released = 10M. Bit 0 = Lo
const uint8_t LPthru  = 1;  // Operated = thru mode. Bit 0 = Hi
const uint8_t LP160   = 2;  // Operated = 160M. Bit 1 = Hi
const uint8_t LP80    = 4;  // Operated = 80M. Bit 2 = Hi
const uint8_t LP60_40 = 8;  // Operated = 60/40M. Bit 3 = Hi
const uint8_t LP30_20 = 16; // Operated = 30/0M Bit 4 = Hi
const uint8_t LP17_15 = 32; // Operated = 17-10M. Bit 5 = Hi


// Available Receive Filters. Edit the bit associated with the filter to suit wiring of circuit

const uint8_t HP160   = 0;  // Released = 160M. Bit 0 = Lo
const uint8_t HPthru  = 1;  // Operated = thru mode; Bit 0 = Hi
const uint8_t HP80    = 2;  // Operated = 80M.  Bit 1 = Hi
const uint8_t HP60_40 = 4;  // Operated = 60/40M  Bit 2 Hi
const uint8_t HP30_20 = 8;  // Operated = 30/20M. Bit 3 Hi
const uint8_t HP17_15 = 16; // Operated = 17-10M. Bit 4 Hi
const uint8_t HP12_10 = 32; // Operated = 17-10M. Bit 5 Hi


// Here we build the map of what filters will be selected from the word formed
// by the bits on USER0,1,2,3 inputs. If 2 filters need to be set, they should
// be or'd together e.g. (LP12_10 || LP17_15)

#if defined(FEATURE_Quisk)

const uint8_t txFilterMap[16] = {
  LP12_10,  // 10 metre Lo Pass in circuit
  LPthru,   // Through pass. i.e. no filters with thru connection
  LP160,    // 10 M LP plus 160 M LP
  LP80,     // 40 M band
  LP60_40,  // 30 M band
  LP30_20,  // 20 M band
  LP17_15,  // 17 M band
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
  LPthru,   // Through pass. i.e. no filters with thru connection
};
const uint8_t rxFilterMap[16] = {
  HP160,    // 160 metre Hi Pass in circuit
  HPthru,   // Through pass. i.e. no filters with thru connection
  HP80,     // 160 M HP plus 80 M HP
  HP60_40,  // 40 M band
  HP30_20,  // 30 and 20 M band
  HP17_15,  // 17 and 15 M band
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection
  HPthru,   // Through pass. i.e. no filters with thru connection  
};
#endif

#if defined(FEATURE_pihpsdr)

#endif

#if defined(FEATURE_Custom_Filter)

#endif
