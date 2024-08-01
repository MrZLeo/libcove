project(
  'libcove',
  'c',
  version: '0.1.0',
  default_options: [
    'c_std=gnu17',
    'warning_level=2',
    'buildtype=debugoptimized',
  ],
)

utils = include_directories('utils')


container_src = files('container/rbtree.c')
container_inc = include_directories('container')
static_library('container', sources: container_src)

minico_inc = include_directories('coroutine')
criterion = subproject('criterion')
criterion_dep = criterion.get_variable('criterion')
subdir('test')
