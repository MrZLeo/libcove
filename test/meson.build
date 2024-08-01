test_framework = dependency('criterion_dep', fallback: ['criterion', 'criterion'])

minico = executable(
  'minico',
  sources: ['minico.c'],
  include_directories: minico_inc,
  c_args: ['-O3'],
  dependencies: [test_framework],
)
test('minico_test', minico)

rbtree = executable(
  'rbtree',
  sources: ['rbtree.c', container_src],
  include_directories: [container_inc, utils],
  # c_args: ['-D__LIBCOVE_RBTREE_DEBUG__'],
  dependencies: [test_framework],

)
test('rbtree_test', rbtree)
