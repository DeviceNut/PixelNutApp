//*********************************************************************************************
// Extended Plugins for the PixelNutLib Library.
//*********************************************************************************************
/*
Copyright (c) 2015-2020, Greg de Valois
Software License Agreement (BSD License)
See license.txt for the terms of this license.
*/
#if defined(PLUGINS_OVERRIDE) && PLUGINS_OVERRIDE

class MyPluginFactory : public PluginFactory
{
public:
  PixelNutPlugin *makePlugin(int plugin)
  {
    switch (plugin)
    {
      #if defined(PLUGIN_SPECTRA) && PLUGIN_SPECTRA
      case 70: return new PNP_Spectra;
      #endif

      #if defined(PLUGIN_PLASMA) && PLUGIN_PLASMA
      case 80: return new PNP_Plasma;
      #endif

      #if defined(PLUGIN_FADER) && PLUGIN_FADER
      case 99: return new PNP_Fader
      #endif

      default: return PluginFactory::makePlugin(plugin);
    }
  }
};

MyPluginFactory pluginFactory = MyPluginFactory();
PluginFactory *pPluginFactory = &pluginFactory;

#endif // PLUGINS_OVERRIDE
//*********************************************************************************************
