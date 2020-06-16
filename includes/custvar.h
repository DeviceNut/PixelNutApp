// PixelNutApp Custom Class: Sets global reference to custom code handler
//========================================================================================
/*
Copyright (c) 2015-2017, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1)) // have logical segments
CustomCode *pCustomCode = &customSegs; // override with segment handler
// note that this will call into bluetooth/wifinet handler if needed
#elif BLE_COMM
CustomCode *pCustomCode = &bluetooth;   // override with bluetooth handler
#elif WIFI_COMM
CustomCode *pCustomCode = &wifinet;     // override wifinet handler
#elif !CUSTOM_CODE
CustomCode customcode;
CustomCode *pCustomCode = &customcode;  // base class handler
#endif

