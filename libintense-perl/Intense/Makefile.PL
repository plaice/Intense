use 5.008;
use ExtUtils::MakeMaker;

push @ARGV, 'CC=g++ -g', 'LD=g++';

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Intense',
    'VERSION_FROM'	=> 'Intense.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Intense.pm', # retrieve abstract from module
       AUTHOR     => 'Paul Swoboda <root@hackomatic.net>') : ()),
    'LIBS'		=> ['-lintense'], # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'INC'		=> '-I.', # e.g., '-I. -I/usr/include/other'
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map' ],
#   'LINKTYPE'          => 'static',
    # Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
);
