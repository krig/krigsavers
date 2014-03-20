#!/usr/bin/env python
import os
import platform
import subprocess
srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'
VERSION_MAJOR_MINOR = ".".join(VERSION.split('.')[0:2])
APPNAME = 'krigsavers'
SAVERS = ['epochsaver', 'circlesaver', 'swarmsaver', 'crisisaver']

def build(bld):
    #print ("Entering into directory " + os.getcwd())

    for sav in SAVERS:
        bld.install_files('${PREFIX}/share/applications/screensavers', sav+'.desktop')
        saver = bld.new_task_gen('cc', 'cprogram')
        saver.source = sav+'.c gs-theme-window.c'
        saver.target = sav
        saver.includes = '.'
        saver.uselib = 'M GTK+-2.0 CAIRO'
        saver.install_path = bld.env['SAVER']
        saver.chmod = 0755

    # variations of epochsaver
    bld.install_files('${PREFIX}/share/applications/screensavers', 'epochcountdown.desktop')
    bld.install_files('${PREFIX}/share/applications/screensavers', 'epochclock.desktop')
    bld.install_files('${PREFIX}/share/applications/screensavers', 'clockblue.desktop')


def configure(conf):
    conf.check_tool('gcc')
    conf.check_tool('g++')
    conf.check(lib='m')
    conf.check_cfg(package='glib-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='gobject-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='gtk+-2.0', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='cairo', args='--cflags --libs', mandatory=1)
    if platform.dist()[0] == 'SuSE':
        conf.env['SAVER'] = '${PREFIX}/lib/gnome-screensaver'
    else:
        conf.env['SAVER'] = os.path.join(conf.check_cfg(package='gnome-screensaver',
                                                        args='--variable=libexecdir',
                                                        mandatory=1).strip(), 'gnome-screensaver')

def set_options(_):
    pass

# don't include these dirs in the
# ./waf dist archive
import Scripting
Scripting.excludes += ['.git', 'debian']
