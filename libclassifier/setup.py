from distutils.core import setup, Extension

splh_module = Extension('splh',
		sources = ['splhmodule.cpp'])

setup ( name = 'SPLH',
	version = '0.01',
	description = 'Sample module for SPLH classification.',
	ext_modules = [ splh_module ])
