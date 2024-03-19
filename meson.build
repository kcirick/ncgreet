project (
  'ncgreet',
  'cpp',
  version: '0.1',
  license: 'MIT',
  meson_version: '>=0.47.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=2',
  ],
)

add_project_arguments(
  [ '-DVERSION="@0@"'.format(meson.project_version()) ],
  language: 'cpp'
)

prefix = get_option('prefix')

ncurses = dependency('ncurses')
form    = dependency('form')
menu    = dependency('menu')
jsoncpp = dependency('jsoncpp')

dependencies = [ncurses, form, menu, jsoncpp]

executable (
  'ncgreet',
  [ 'Main.cc',
    'Authenticator.cc',
    'Configurator.cc',
    'Greeter.cc',
  ],
  dependencies: dependencies,
  install: true
)
