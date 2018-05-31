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
      '../libdvbpsi/src',
      '../libdvbpsi/src/tables',
      '../libdvbpsi/src/descriptors',
      '../libdvbpsi'
    ],
    'defines': [
      'PIC',
      'HAVE_CONFIG_H'
    ],
  },

  'targets': [
    # libdvbpsi
    {
      'target_name': 'dvbpsi',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        '../libdvbpsi/src/dvbpsi.c',
        '../libdvbpsi/src/psi.c',
        '../libdvbpsi/src/demux.c',
        '../libdvbpsi/src/descriptor.c',
        '../libdvbpsi/src/tables/pat.c',
        '../libdvbpsi/src/tables/pmt.c',
        '../libdvbpsi/src/tables/sdt.c',
        '../libdvbpsi/src/tables/eit.c',
#       '../libdvbpsi/src/tables/cat.c',
        '../libdvbpsi/src/tables/nit.c',
        '../libdvbpsi/src/tables/tot.c',
#       '../libdvbpsi/src/tables/sis.c',
#       '../libdvbpsi/src/tables/bat.c',
#       '../libdvbpsi/src/tables/rst.c',
        '../libdvbpsi/src/tables/atsc_vct.c',
        '../libdvbpsi/src/tables/atsc_stt.c',
        '../libdvbpsi/src/tables/atsc_eit.c',
        '../libdvbpsi/src/tables/atsc_ett.c',
        '../libdvbpsi/src/tables/atsc_mgt.c',
#       '../libdvbpsi/src/descriptors/dr_02.c',
#       '../libdvbpsi/src/descriptors/dr_03.c',
#       '../libdvbpsi/src/descriptors/dr_04.c',
#       '../libdvbpsi/src/descriptors/dr_05.c',
#       '../libdvbpsi/src/descriptors/dr_06.c',
#       '../libdvbpsi/src/descriptors/dr_07.c',
#       '../libdvbpsi/src/descriptors/dr_08.c',
#       '../libdvbpsi/src/descriptors/dr_09.c',
        '../libdvbpsi/src/descriptors/dr_0a.c',
#       '../libdvbpsi/src/descriptors/dr_0b.c',
#       '../libdvbpsi/src/descriptors/dr_0c.c',
#       '../libdvbpsi/src/descriptors/dr_0d.c',
#       '../libdvbpsi/src/descriptors/dr_0e.c',
#       '../libdvbpsi/src/descriptors/dr_0f.c',
#       '../libdvbpsi/src/descriptors/dr_10.c',
#       '../libdvbpsi/src/descriptors/dr_11.c',
#       '../libdvbpsi/src/descriptors/dr_12.c',
#       '../libdvbpsi/src/descriptors/dr_13.c',
#       '../libdvbpsi/src/descriptors/dr_14.c',
#       '../libdvbpsi/src/descriptors/dr_1b.c',
#       '../libdvbpsi/src/descriptors/dr_1c.c',
#       '../libdvbpsi/src/descriptors/dr_24.c',
#       '../libdvbpsi/src/descriptors/dr_40.c',
#       '../libdvbpsi/src/descriptors/dr_41.c',
#       '../libdvbpsi/src/descriptors/dr_42.c',
#       '../libdvbpsi/src/descriptors/dr_43.c',
#       '../libdvbpsi/src/descriptors/dr_44.c',
#       '../libdvbpsi/src/descriptors/dr_45.c',
#       '../libdvbpsi/src/descriptors/dr_47.c',
        '../libdvbpsi/src/descriptors/dr_48.c',
#       '../libdvbpsi/src/descriptors/dr_49.c',
#       '../libdvbpsi/src/descriptors/dr_4a.c',
#       '../libdvbpsi/src/descriptors/dr_4b.c',
#       '../libdvbpsi/src/descriptors/dr_4c.c',
        '../libdvbpsi/src/descriptors/dr_4d.c',
        '../libdvbpsi/src/descriptors/dr_4e.c',
#       '../libdvbpsi/src/descriptors/dr_4f.c',
#       '../libdvbpsi/src/descriptors/dr_50.c',
#       '../libdvbpsi/src/descriptors/dr_52.c',
#       '../libdvbpsi/src/descriptors/dr_53.c',
#       '../libdvbpsi/src/descriptors/dr_54.c',
#       '../libdvbpsi/src/descriptors/dr_55.c',
#       '../libdvbpsi/src/descriptors/dr_56.c',
#       '../libdvbpsi/src/descriptors/dr_58.c',
#       '../libdvbpsi/src/descriptors/dr_59.c',
#       '../libdvbpsi/src/descriptors/dr_5a.c',
        '../libdvbpsi/src/descriptors/dr_62.c',
#       '../libdvbpsi/src/descriptors/dr_66.c',
#       '../libdvbpsi/src/descriptors/dr_69.c',
#       '../libdvbpsi/src/descriptors/dr_73.c',
#       '../libdvbpsi/src/descriptors/dr_76.c',
#       '../libdvbpsi/src/descriptors/dr_7c.c',
        '../libdvbpsi/src/descriptors/dr_81.c',
        '../libdvbpsi/src/descriptors/dr_83.c',
        '../libdvbpsi/src/descriptors/dr_86.c',
#       '../libdvbpsi/src/descriptors/dr_8a.c',
        '../libdvbpsi/src/descriptors/dr_a0.c',
        '../libdvbpsi/src/descriptors/dr_a1.c',
      ],
      'conditions': [
        ['OS=="mac"',
          {
            'xcode_settings': {
              'WARNING_CFLAGS': [
                '-Wno-deprecated-declarations'
              ]
            }
          }
        ]
      ],
      'cflags!': ['-Wdeprecated-declarations','-Wimplicit-function-declaration'],
      'cflags+': ['-Wno-deprecated-declarations','-Wno-implicit-function-declaration','-std=c99'],
    },
  ]
}
