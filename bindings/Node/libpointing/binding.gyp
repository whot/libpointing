{
  "targets": [
    {
      "target_name": "pointing",
      "sources": [ 
      	"pointing/pointing.cc", 
      	"pointing/npointingdevice.cc",
      	"pointing/ndisplaydevice.cc",
      	"pointing/ntransferfunction.cc",
      	"pointing/npointingdevicemanager.cc",
      	"pointing/ndisplaydevicemanager.cc",
      	"pointing/nsystempointeracceleration.cc",
      ],
      'conditions': [
			['OS=="mac"', {
				"link_settings": {
					'libraries': [
					   '-lpointing',
					   '-L/usr/local/lib'
					 ]
				 },
				"include_dirs": [
					'/usr/local/include',
					"<!(node -e \"require('nan')\")"
				],
				"cflags": [ "-mmacosx-version-min=10.6" ],
			}],
			['OS=="linux"', {
				"link_settings": {
				'libraries': [
				   '-lpointing',
				   '-L/usr/local/lib'
				 ]},
				"include_dirs": [
					'/usr/local/include',
					"<!(node -e \"require('nan')\")"
				]
			}],
			['OS=="win"', {
				"link_settings": {
					"conditions": [
						["target_arch == 'ia32'", {
							'libraries': [
								'-lsetupapi',
								'-lhid',
								'-luser32',
								'-ladvapi32',
								'../../../../pointing/Release/pointing.lib'
						 	]
						}],
						["target_arch == 'x64'", {
							'libraries': [
								'-lsetupapi',
								'-lhid',
								'-luser32',
								'-ladvapi32',
								'../../../../pointing/x64/Release/pointing.lib'
						 	]
						}]
					]
				 },
				"include_dirs": [
					'../../..',
					"<!(node -e \"require('nan')\")"
				],
				'msvs_settings': {
				  	'VCCLCompilerTool': {
				    	'RuntimeLibrary': 2, # multi threaded DLL
				  	},
				},
				"msvs_disabled_warnings": [ 4244, 4267 ],
			}],
		],
    }
  ]
}
