// PixelNutApp Custom Class: Sets global reference to custom code handler
//========================================================================================
/*
Copyright (c) 2015-2024, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/

#if (!STRANDS_MULTI && (SEGMENT_COUNT > 1)) // have logical segments
CustomCode *pCustomCode = &customSegs; // override with segment handler
// note that this will call into bluetooth/wifi handler if needed

#elif BLE_BLUEFRUIT
CustomCode *pCustomCode = &bleBluefruit;    // override with bluefruit handler

#elif BLE_ESP32
CustomCode *pCustomCode = &bleEsp32;        // override with bluetooth handler

#elif WIFI_SOFTAP
CustomCode *pCustomCode = &wifiSAP;         // override with wifi SoftAP handler

#elif !(defined(CUSTOM_CODE) && CUSTOM_CODE)
CustomCode customcode;
CustomCode *pCustomCode = &customcode;  // base class handler

#endif
