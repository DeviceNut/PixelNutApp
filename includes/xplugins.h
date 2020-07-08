//*********************************************************************************************
// Extended Plugins for the PixelNutLib Library.
//*********************************************************************************************
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
  
      default: return PluginFactory::makePlugin(plugin);
    }
  }
};

MyPluginFactory pluginFactory = MyPluginFactory();
PluginFactory *pPluginFactory = &pluginFactory;

#endif // PLUGINS_OVERRIDE
//*********************************************************************************************
