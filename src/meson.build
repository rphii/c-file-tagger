
sources = [
  'cft.c',
  'info.c',
  'lookup.c',
  'main.c',
  'platform.c',
  'vector.c',
  ]

cc = meson.get_compiler('c')

dep_ft2 = dependency('freetype2', required: true)
dep_rlc = dependency('rlc', required: true)
dep_rlso = dependency('rlso', required: true)
dep_rlarg = dependency('rlarg', required: true)
dep_m = cc.find_library('m', required: false)

cft = executable('cft',
  sources,
  dependencies: [dep_ft2, dep_rlc, dep_rlso, dep_rlarg, dep_m],
  install: true,
  )

