test_framework = dependency('criterion_dep', fallback: ['criterion', 'criterion'])

minico = executable(
  'minico',
  sources: ['minico.c'],
  include_directories: minico_inc,
  c_args: ['-O3'],
  dependencies: [test_framework],
)
test('minico_test', minico)
