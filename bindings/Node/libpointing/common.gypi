{
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Release': {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 2 # multi threaded DLL
          }
        }
      }
    }
  }
}