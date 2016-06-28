from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import platform

libpointing='../../../'

if platform.system() == 'Darwin':
    ext_modules = [Extension(
                        "pylibpointing", 
                        ["pylibpointing.pyx"],
                        language="c++",   
                        libraries=['pointing'],
                        include_dirs=[libpointing],
                        library_dirs=["./",libpointing+"pointing"],
                        extra_compile_args=["-stdlib=libc++", "-mmacosx-version-min=10.7", "-std=c++11"],
                        extra_link_args=["-mmacosx-version-min=10.7", "-framework", "CoreGraphics"],
                        )]
elif platform.system() == 'Linux':
    ext_modules = [Extension(
                         "pylibpointing", 
                         ["pylibpointing.pyx"],
                         language="c++",   
                         libraries=['pointing', "stdc++", "udev", "X11", "Xrandr"],
                         include_dirs=[
                            libpointing
                            ],
                         library_dirs=["./"]
                         )    
                         ]
else:
    ext_modules = [Extension(
                         "pylibpointing", 
                         ["pylibpointing.pyx"],
                         language="c++",   
                         libraries=['pointing', "stdc++"],
                         include_dirs=[
                            libpointing
                            ],
                         library_dirs=["./"]
                         )    
                         ]

setup(
  name = 'pylibpointing',
  cmdclass = {'build_ext': build_ext},
  ext_modules = ext_modules
)
