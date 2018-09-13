{
  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
          },
        },
      }
    },
    'msvs_settings': {
      'VCLinkerTool': {
        'GenerateDebugInformation': 'true',
      },
    },
    'include_dirs': [
      'usr/include',
      'usr/include/dvbtee',
      'usr/include/dvbpsi',
      'libdvbtee',
      'libdvbtee/decode',
      'libdvbtee/decode/table',
      'libdvbtee/decode/descriptor',
      '../libdvbtee'
    ],
    'defines': [
      'PIC',
      'FORCE_DECODER_LINKAGE'
    ],
  },

  'targets': [
    # libdvbtee_parser
    {
      'target_name': 'dvbtee_parser',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        'libdvbtee/atsctext.cpp',
#       'libdvbtee/channels.cpp',
#       'libdvbtee/curlhttpget.cpp',
        'libdvbtee/decode.cpp',
#       'libdvbtee/demux.cpp',
#       'libdvbtee/dvb-vb2.cpp',
#       'libdvbtee/feed.cpp',
        'libdvbtee/functions.cpp',
#       'libdvbtee/hdhr_tuner.cpp',
#       'libdvbtee/hlsfeed.cpp',
#       'libdvbtee/linuxtv_tuner.cpp',
#       'libdvbtee/listen.cpp',
        'libdvbtee/log.cpp',
#       'libdvbtee/output.cpp',
        'libdvbtee/parse.cpp',
#       'libdvbtee/rbuf.cpp',
        'libdvbtee/stats.cpp',
#       'libdvbtee/tune.cpp',

        'libdvbtee/value/value.cpp',
        'libdvbtee/value/object.cpp',
        'libdvbtee/value/array.cpp',
        'libdvbtee/value/utf8strip.cpp',

        'libdvbtee/decode/decoder.cpp',
        'libdvbtee/decode/table/table.cpp',
        'libdvbtee/decode/descriptor/descriptor.cpp',

        'libdvbtee/decode/table/tabl_00.cpp',
        'libdvbtee/decode/table/tabl_02.cpp',
        'libdvbtee/decode/table/tabl_40.cpp',
        'libdvbtee/decode/table/tabl_42.cpp',
        'libdvbtee/decode/table/tabl_4e.cpp',
        'libdvbtee/decode/table/tabl_70.cpp',
        'libdvbtee/decode/table/tabl_c7.cpp',
        'libdvbtee/decode/table/tabl_c8.cpp',
        'libdvbtee/decode/table/tabl_cb.cpp',
        'libdvbtee/decode/table/tabl_cc.cpp',
        'libdvbtee/decode/table/tabl_cd.cpp',

        'libdvbtee/decode/descriptor/desc_0a.cpp',
        'libdvbtee/decode/descriptor/desc_48.cpp',
        'libdvbtee/decode/descriptor/desc_4d.cpp',
        'libdvbtee/decode/descriptor/desc_4e.cpp',

        'libdvbtee/decode/descriptor/desc_62.cpp',
        'libdvbtee/decode/descriptor/desc_83.cpp',

        'libdvbtee/decode/descriptor/desc_81.cpp',
        'libdvbtee/decode/descriptor/desc_86.cpp',
        'libdvbtee/decode/descriptor/desc_a0.cpp',
        'libdvbtee/decode/descriptor/desc_a1.cpp',
      ],
      "dependencies": [
        'deps/libdvbpsi.gyp:dvbpsi'
      ],
      'conditions': [
        ['OS=="mac"',
          {
            'defines': [
              'USE_WSTRING_CONVERT'
            ],
            'link_settings': {
              'libraries': [
                '-liconv'
              ]
            },
            'xcode_settings': {
              'MACOSX_DEPLOYMENT_TARGET': '10.7',
              'CLANG_CXX_LIBRARY': 'libc++',
              'OTHER_LDFLAGS': [
                '-L/opt/local/lib'
              ],
              'WARNING_CFLAGS': [
                '-Wno-unused-variable',
                '-Wno-deprecated-declarations'
              ],
              'GCC_ENABLE_CPP_RTTI': '-frtti'
            }
          }
        ]
      ],
      'cflags_cc!': [
        '-fno-rtti',
        '-Wformat-overflow',
        '-Wunused-variable',
        '-Wdeprecated-declarations'
      ],
      'cflags_cc+': [
        '-frtti',
        '-Wno-format-overflow',
        '-Wno-unused-variable',
        '-Wno-deprecated-declarations'
      ],
    },
  ]
}
