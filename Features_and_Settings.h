// Features are chosen here. Uncomment the desired features

//#define FEATURE_I2C_LCD
#define FEATURE_Quisk
//#define FEATURE_pihpsdr
//#define FEATURE_Custom_Filter

// Here we build the map of what filters will be selected from the word formed
// by the bits on USER0,1,2,3 inputs

#if defined(FEATURE_Quisk)

const uint8_t TxFilterMap[16] = {
  0, // 160 M band
  1, // 80 M band
  2, // 60 M band
  3, // 40 M band
  4, // 30 M band
  5, // 20 M band
  6, // 17 M band
  7, // 15 M band
  8, // 12 M band
  9, // 10 M band
  0, // Not defined
  0, // Not defined
  0, // Not defined
  0, // Not defined
  0, // Not defined
  0  // Not defined
};
const uint8_t RxFilterMap[16] = {
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15
};

#endif

#if defined(FEATURE_pihpsdr)

#endif

#if defined(FEATURE_Custom_Filter)

#endif
