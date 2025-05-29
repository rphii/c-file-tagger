
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
dep_rphiic = dependency('rphiic', required: true)
dep_glfw3 = dependency('glfw3', required: true)
dep_m = cc.find_library('m', required: false)

cft = executable('cft',
  sources,
  dependencies: [dep_ft2, dep_rphiic, dep_glfw3, dep_m],
  install: true,
  )

