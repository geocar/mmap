{
  "targets": [
    {
      "target_name": "mmap",
      "sources": [ "mmap.cpp" ],
      'libraries': [ '-L/Users/greg/git/mmap/lib', '-v', '-lc++' ],
      'link-settings': { 'libraries!': [ '-platform-version', '-lstdc++' ] },
      'xcode_settings': { 
        'MACOSX_DEPLOYMENT_TARGET': '10.9',
	'OTHER_CPLUSPLUSFLAGS' : ['-stdlib=libc++', '-Wall', '-mmacosx-version-min=10.9' ],
        'OTHER_LDFLAGS': ['-stdlib=libc++'],
       },
    }
  ]
}
